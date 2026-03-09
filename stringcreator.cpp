#include "stringcreator.hpp"

std::string secondsToTime(int seconds) {
    int hours = seconds / 3600;
    int minutes = (seconds / 60) % 60;
    seconds = seconds % 60;
    std::string out = "";
    if (hours > 0) out += std::to_string(hours) + "h ";
    if (minutes > 0) out += std::to_string(minutes) + "m ";
    out += std::to_string(seconds) + "s ";
    return out;
}
std::string generateProgressBar(float percent, int width) {
    int filledChars = static_cast<int>(static_cast<float>(width) * percent);
    std::string output = "[";
    output.append(filledChars, '=');
    output.append(width - filledChars, ' ');
    output.append("]");
    return output;
}
