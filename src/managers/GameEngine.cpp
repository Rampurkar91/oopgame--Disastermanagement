#include "../../include/managers/GameEngine.hpp"
#include "../../include/managers/SaveManager.hpp"
#include <algorithm>
#include <exception>
#include <sstream>

namespace drc {

GameEngine::GameEngine() { newMission(1); }

void GameEngine::spawnDefaultEntities() {
    vehicles_ = VehicleFactory::createDefaultFleet();
    civilians_.clear();
    // Original spec: P1..P5 with fixed health/priority.
    civilians_.emplace_back(1, "P1", Position{2,2}, 25, 10);
    civilians_.emplace_back(2, "P2", Position{4,4}, 75, 4);
    civilians_.emplace_back(3, "P3", Position{6,6}, 45, 8);
    civilians_.emplace_back(4, "P4", Position{3,10}, 55, 7);
    civilians_.emplace_back(5, "P5", Position{7,7}, 30, 9);
    hospitals_.clear();
    hospitals_.push_back(std::make_unique<Hospital>(1, "Hospital 1", Position{0,11}, 4));
    hospitals_.push_back(std::make_unique<Hospital>(2, "Hospital 2", Position{8,11}, 3));
    fireStation_ = std::make_unique<FireStation>(1, "Fire Station", Position{2,8});
}

void GameEngine::newMission(int level) {
    level_ = std::max(1, std::min(5, level));
    config_.level = level_;
    config_.name = "Level " + std::to_string(level_);
    // Difficulty scaling (classic level 1 preserves original values).
    switch (level_) {
        case 1: config_.timeLimit = 80; config_.fuelBonus = 0; config_.supplyBonus = 0; break;
        case 2: config_.timeLimit = 80; config_.fuelBonus = 0; config_.supplyBonus = 0; break;
        case 3: config_.timeLimit = 70; config_.fuelBonus = 0; config_.supplyBonus = 0; break;
        case 4: config_.timeLimit = 60; config_.fuelBonus = 0; config_.supplyBonus = 0; break;
        case 5: config_.timeLimit = 50; config_.fuelBonus = 0; config_.supplyBonus = 0; break;
    }
    map_.loadLevel(level_);
    spawnDefaultEntities();
    score_ = 0;
    timeLeft_ = config_.timeLimit;
    paused_ = false;
    selectedVehicleId_ = 1;
    hasRoute_ = false;
    lastRoute_ = DijkstraResult{};
}

void GameEngine::reset() { newMission(level_); }

Vehicle* GameEngine::findVehicle(int id) {
    for (auto& v : vehicles_) if (v->id() == id) return v.get();
    return nullptr;
}
Civilian* GameEngine::findCivilian(int id) {
    for (auto& c : civilians_) if (c.id() == id) return &c;
    return nullptr;
}
Hospital* GameEngine::findHospital(int id) {
    for (auto& h : hospitals_) if (h->id() == id) return h.get();
    return nullptr;
}
Vehicle* GameEngine::selectedVehicle() { return findVehicle(selectedVehicleId_); }
const Vehicle* GameEngine::selectedVehicle() const {
    for (auto& v : vehicles_) if (v->id() == selectedVehicleId_) return v.get();
    return nullptr;
}
void GameEngine::selectVehicle(int id) {
    if (!findVehicle(id) || selectedVehicleId_ == id) return;
    selectedVehicleId_ = id;
    // A route belongs to the vehicle that planned it; never reuse it for another unit.
    hasRoute_ = false;
    lastRoute_ = DijkstraResult{};
}

int GameEngine::rescuedCount() const {
    int n = 0; for (auto& c : civilians_) if (c.isRescued()) ++n; return n;
}
int GameEngine::remainingCount() const { return (int)civilians_.size() - rescuedCount(); }

DijkstraResult GameEngine::planRoute(Position goal) {
    Vehicle* v = selectedVehicle();
    if (!v) return DijkstraResult{};
    lastGoal_ = goal;
    lastRoute_ = Dijkstra::findPath(map_, v->position(), goal, routeMode_, *v);
    hasRoute_ = lastRoute_.found;
    return lastRoute_;
}

bool GameEngine::moveVehicleAlongRoute() {
    Vehicle* v = selectedVehicle();
    if (!v || !hasRoute_ || lastRoute_.path.size() < 2) return false;
    // Move one step along the planned path (fuel checked per step).
    // Find current index in path.
    size_t idx = 0;
    bool foundCurrentPosition = false;
    for (size_t i = 0; i < lastRoute_.path.size(); ++i) {
        if (lastRoute_.path[i] == v->position()) { idx = i; foundCurrentPosition = true; break; }
    }
    if (!foundCurrentPosition || idx + 1 >= lastRoute_.path.size()) {
        hasRoute_ = false;
        return false;
    }
    Position next = lastRoute_.path[idx + 1];
    if (!v->hasFuelFor(1)) { v->setStatus(VehicleStatus::OutOfFuel); return false; }
    v->consumeFuel(1);
    v->setPosition(next);
    v->setStatus(VehicleStatus::Moving);
    if (next == lastGoal_) { v->setStatus(VehicleStatus::Idle); hasRoute_ = false; }
    return true;
}

RescueResult GameEngine::rescueCivilian(int civilianId) {
    RescueResult r;
    Vehicle* v = selectedVehicle();
    Civilian* c = findCivilian(civilianId);
    if (!v || !c) { r.message = "Invalid vehicle or civilian."; return r; }
    if (c->isRescued()) { r.message = "Civilian already rescued."; return r; }
    if (!(v->position() == c->position())) { r.message = "Vehicle must reach the civilian first."; return r; }
    c->setStatus(CivilianStatus::Rescued);
    c->setAssignedVehicleId(v->id());
    v->setStatus(VehicleStatus::Rescuing);
    r.success = true;
    r.scoreGained = c->priority(); // rescue score based on priority
    score_ += r.scoreGained;
    r.message = "Rescued " + c->name() + " (+" + std::to_string(r.scoreGained) + ")";
    return r;
}

RescueResult GameEngine::deliverToHospital(int civilianId, int hospitalId) {
    RescueResult r;
    Vehicle* v = selectedVehicle();
    Civilian* c = findCivilian(civilianId);
    Hospital* h = findHospital(hospitalId);
    if (!v || !c || !h) { r.message = "Invalid delivery target."; return r; }
    if (c->status() != CivilianStatus::Rescued) { r.message = "Civilian must be rescued first."; return r; }
    if (c->assignedVehicleId() >= 0 && c->assignedVehicleId() != v->id()) {
        r.message = "The assigned rescue vehicle must deliver this civilian."; return r;
    }
    if (!(v->position() == h->position())) { r.message = "Vehicle must reach the hospital first."; return r; }
    if (!h->hasSpace()) { r.message = h->name() + " is full."; return r; }
    h->admit();
    c->setStatus(CivilianStatus::Admitted);
    c->setHospitalId(h->id());
    v->setStatus(VehicleStatus::Delivering);
    r.success = true;
    r.scoreGained = 50; // hospital admission bonus
    score_ += r.scoreGained;
    r.message = "Admitted " + c->name() + " to " + h->name() + " (+" + std::to_string(r.scoreGained) + ")";
    return r;
}

RescueResult GameEngine::treatCivilian(int civilianId) {
    RescueResult r;
    Vehicle* v = selectedVehicle();
    Civilian* c = findCivilian(civilianId);
    if (!v || !c) { r.message = "Invalid treatment target."; return r; }
    if (c->status() == CivilianStatus::Admitted || c->status() == CivilianStatus::Rescued) {
        r.message = "This civilian has already been rescued."; return r;
    }
    if (c->status() == CivilianStatus::Treated) { r.message = "This civilian has already been treated."; return r; }
    if (!(v->position() == c->position())) { r.message = "Vehicle must reach the civilian first."; return r; }
    if (!v->useSupply()) { r.message = "No medical supplies left."; return r; }
    c->setHealth(std::min(100, c->health() + 20));
    c->setStatus(CivilianStatus::Treated);
    r.success = true;
    r.scoreGained = 20; // medical treatment bonus
    score_ += r.scoreGained;
    r.message = "Treated " + c->name() + " (+" + std::to_string(r.scoreGained) + ")";
    return r;
}

bool GameEngine::refuelSelectedVehicle() {
    Vehicle* v = selectedVehicle();
    if (!v) return false;
    if (!(v->position() == fireStation_->position())) return false;
    v->refuel();
    v->setStatus(VehicleStatus::Refueling);
    return true;
}

void GameEngine::tick() {
    if (paused_ || isVictory() || isDefeat()) return;
    if (timeLeft_ > 0) --timeLeft_;
}

bool GameEngine::isVictory() const { return remainingCount() == 0; }
bool GameEngine::isDefeat() const { return timeLeft_ <= 0 && remainingCount() > 0; }

std::vector<Civilian*> GameEngine::recommendedTargets() {
    std::vector<Civilian*> out;
    for (auto& c : civilians_) if (!c.isRescued()) out.push_back(&c);
    std::sort(out.begin(), out.end(), [](Civilian* a, Civilian* b){ return a->urgencyScore() > b->urgencyScore(); });
    return out;
}

std::string GameEngine::statusSummary() const {
    std::ostringstream ss;
    ss << "Level " << level_ << " | Score " << score_ << " | Time " << timeLeft_
       << " | Rescued " << rescuedCount() << "/" << civilians_.size();
    return ss.str();
}

bool GameEngine::saveGame(const std::string& path) const {
    std::ostringstream ss;
    ss << "level=" << level_ << "\nscore=" << score_ << "\ntime=" << timeLeft_ << "\n";
    for (auto& v : vehicles_) ss << "vehicle " << v->id() << " " << v->position().row << " " << v->position().col << " " << v->fuel() << " " << v->supplies() << "\n";
    for (auto& c : civilians_) ss << "civilian " << c.id() << " " << c.health() << " " << (int)c.status()
                                   << " " << c.assignedVehicleId() << " " << c.hospitalId() << "\n";
    for (auto& h : hospitals_) ss << "hospital " << h->id() << " " << h->occupied() << "\n";
    return SaveManager::saveText(path, ss.str());
}

bool GameEngine::loadGame(const std::string& path) {
    std::string content;
    if (!SaveManager::loadText(path, content)) return false;
    std::istringstream in(content);
    std::string line;
    int lvl = 1, sc = 0, tm = 80;
    struct VSave {int id,r,c,f,s;}; std::vector<VSave> vs;
    struct CSave {int id,h,st,vehicleId,hospitalId;}; std::vector<CSave> cs;
    struct HSave {int id,occ;}; std::vector<HSave> hs;
    try {
        while (std::getline(in, line)) {
            std::istringstream ls(line);
            std::string key; ls >> key;
            if (key.rfind("level=", 0) == 0) lvl = std::stoi(key.substr(6));
            else if (key.rfind("score=", 0) == 0) sc = std::stoi(key.substr(6));
            else if (key.rfind("time=", 0) == 0) tm = std::stoi(key.substr(5));
            else if (key == "vehicle") {
                VSave v{};
                if (!(ls >> v.id >> v.r >> v.c >> v.f >> v.s)) return false;
                vs.push_back(v);
            } else if (key == "civilian") {
                CSave c{}; c.vehicleId = -1; c.hospitalId = -1;
                if (!(ls >> c.id >> c.h >> c.st)) return false;
                ls >> c.vehicleId >> c.hospitalId; // optional fields keep older saves readable
                cs.push_back(c);
            } else if (key == "hospital") {
                HSave h{};
                if (!(ls >> h.id >> h.occ)) return false;
                hs.push_back(h);
            }
        }
    } catch (const std::exception&) {
        return false;
    }
    if (lvl < 1 || lvl > 5 || tm < 0 || vs.size() < 4 || cs.size() < 5 || hs.size() < 2)
        return false;
    DisasterMap validationMap;
    validationMap.loadLevel(lvl);
    for (const auto& v : vs)
        if (v.id < 1 || v.id > 4 || !validationMap.inBounds({v.r, v.c})) return false;
    for (const auto& c : cs)
        if (c.id < 1 || c.id > 5 || c.h < 0 || c.h > 100 ||
            c.st < (int)CivilianStatus::Waiting || c.st > (int)CivilianStatus::Treated ||
            c.vehicleId < -1 || c.vehicleId > 4 || c.hospitalId < -1 || c.hospitalId > 2) return false;
    for (const auto& h : hs)
        if (h.id < 1 || h.id > 2 || h.occ < 0) return false;
    newMission(lvl);
    score_ = sc; timeLeft_ = tm;
    for (auto& s : vs) if (Vehicle* v = findVehicle(s.id)) {
        v->setPosition({s.r, s.c});
        v->restoreResources(s.f, s.s);
    }
    for (auto& s : cs) if (Civilian* c = findCivilian(s.id)) {
        c->setHealth(s.h);
        if (s.st >= (int)CivilianStatus::Waiting && s.st <= (int)CivilianStatus::Treated)
            c->setStatus((CivilianStatus)s.st);
        c->setAssignedVehicleId(s.vehicleId);
        c->setHospitalId(s.hospitalId);
    }
    for (auto& s : hs) if (Hospital* h = findHospital(s.id)) h->setOccupied(s.occ);
    return true;
}

} // namespace drc
