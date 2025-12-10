#pragma once
namespace yuki {
class Time {
public:
    Time();
    void update();
    float deltaTime() const;
    float elapsed() const;
private:
    double previousTime;
    double currentTime;
    float dt;
    double startTime;
};
}
