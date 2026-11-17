#pragma once
#include "../core/Position.hpp"
#include <string>

namespace drc {

// Abstract base (preserved original class name).
class Facility {
public:
    Facility(int id, std::string name, Position pos) : id_(id), name_(std::move(name)), position_(pos) {}
    virtual ~Facility() = default;
    int id() const { return id_; }
    const std::string& name() const { return name_; }
    Position position() const { return position_; }
    virtual std::string type() const = 0;
protected:
    int id_;
    std::string name_;
    Position position_;
};

class Hospital : public Facility {
public:
    Hospital(int id, std::string name, Position pos, int capacity)
        : Facility(id, std::move(name), pos), capacity_(capacity), occupied_(0) {}
    std::string type() const override { return "Hospital"; }
    int capacity() const { return capacity_; }
    int occupied() const { return occupied_; }
    int freeBeds() const { return capacity_ - occupied_; }
    bool hasSpace() const { return occupied_ < capacity_; }
    bool admit() {
        if (!hasSpace()) return false;
        ++occupied_;
        return true;
    }
    void setOccupied(int n) { occupied_ = std::max(0, std::min(capacity_, n)); }
private:
    int capacity_;
    int occupied_;
};

class FireStation : public Facility {
public:
    FireStation(int id, std::string name, Position pos) : Facility(id, std::move(name), pos) {}
    std::string type() const override { return "FireStation"; }
    bool available() const { return available_; }
    void setAvailable(bool a) { available_ = a; }
private:
    bool available_ = true;
};

} // namespace drc
