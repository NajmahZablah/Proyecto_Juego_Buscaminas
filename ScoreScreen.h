#ifndef SCORESCREEN_H
#define SCORESCREEN_H
#pragma once
#include <SFML/Graphics.hpp>
#include <vector>
#include <string>
#include <map>
#include "UIHelpers.h"

// Forward declaration (SOLUCIONA TU ERROR)
enum class GameScreen;

struct ScoreEntry {
    std::string playerName;
    float tiempo;
};

class ScoreScreen {
private:
    std::map<std::string, std::vector<ScoreEntry>> scores;
    sf::Font font;
    sf::Text titleText;
    sf::Text scoreTexts[10];
    Button btnBack;

public:
    ScoreScreen();

    void loadScores();
    void draw(sf::RenderWindow &window, const std::string &nivel);
    void addScore(const std::string &playerName, float tiempo, const std::string &nivel);

    GameScreen handleEvent(const sf::Event& e);
    void setupButtons(float windowWidth, float windowHeight);
};
#endif
