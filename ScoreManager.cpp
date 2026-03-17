#include "ScoreManager.h"

std::string ScoreManager::getFileName(const std::string& level) {
    if (level == "principiante") return "scores_beginner.dat";
    if (level == "intermedio")  return "scores_intermediate.dat";
    if (level == "experto")     return "scores_expert.dat";
    return "scores.dat";
}

void ScoreManager::addScore(const std::string& username, float time, const std::string& level) {
    std::ofstream file(getFileName(level), std::ios::app);
    if(file.is_open()) {
        file << username << " " << time << "\n";
        file.close();
    }
}

std::vector<ScoreEntry> ScoreManager::getTopScores(const std::string& level, int topN) {
    std::vector<ScoreEntry> scores;
    std::ifstream file(getFileName(level));
    if(file.is_open()) {
        std::string name;
        float t;
        while(file >> name >> t) {
            scores.push_back({name, t});
        }
        file.close();
    }
    std::sort(scores.begin(), scores.end(), [](const ScoreEntry& a, const ScoreEntry& b) {
        return a.time < b.time; // menor tiempo es mejor
    });

    if(scores.size() > topN) scores.resize(topN);
    return scores;
}
