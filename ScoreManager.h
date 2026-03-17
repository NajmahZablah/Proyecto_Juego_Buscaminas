#ifndef SCOREMANAGER_H
#define SCOREMANAGER_H

#include <string>
#include <vector>
#include <fstream>
#include <algorithm>

struct ScoreEntry {
    std::string username;
    float time; // tiempo en segundos
};

class ScoreManager {
public:
    void addScore(const std::string& username, float time, const std::string& level);
    std::vector<ScoreEntry> getTopScores(const std::string& level, int topN = 10);

private:
    std::string getFileName(const std::string& level);
};

#endif
