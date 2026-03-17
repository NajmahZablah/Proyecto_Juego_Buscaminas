// ScoreScreen.cpp
#include "ScoreScreen.h"
#include "ScreenManager.h" // IMPORTANTE (aqui SI existe GameScreen)
#include <algorithm>
#include <sstream>

ScoreScreen::ScoreScreen() {
    font.loadFromFile("C:/Windows/Fonts/arial.ttf");

    titleText.setFont(font);
    titleText.setCharacterSize(30);
    titleText.setFillColor(sf::Color::Yellow);
    titleText.setString("Ranking");
    titleText.setPosition(20.f, 10.f);

    setupButtons(1200.f, 700.f);
    loadScores();
}

void ScoreScreen::setupButtons(float windowWidth, float windowHeight) {
    float cx = windowWidth / 2.f;
    float y  = windowHeight - 80.f; // abajo centrado

    btnBack.setup(font, "< Volver", {cx, y}, {200.f, 45.f}, 18);

    btnBack.normalColor = {20, 20, 60, 220};
    btnBack.hoverColor  = {40, 40, 120, 240};
}

void ScoreScreen::loadScores() {
    scores["Principiante"] = {};
    scores["Intermedio"] = {};
    scores["Experto"] = {};
}

void ScoreScreen::addScore(const std::string &playerName, float tiempo, const std::string &nivel) {
    if (tiempo <= 0.f) return;

    scores[nivel].push_back({playerName, tiempo});

    std::sort(scores[nivel].begin(), scores[nivel].end(),
              [](const ScoreEntry &a, const ScoreEntry &b) {
                  return a.tiempo < b.tiempo;
              });

    if (scores[nivel].size() > 10)
        scores[nivel].resize(10);
}

void ScoreScreen::draw(sf::RenderWindow &window, const std::string &nivel) {
    window.clear(sf::Color(10, 15, 50));
    window.draw(titleText);

    int y = 80;
    int i = 0;

    for (auto &entry : scores[nivel]) {
        if (entry.tiempo <= 0) continue;

        scoreTexts[i].setFont(font);
        scoreTexts[i].setCharacterSize(22);
        scoreTexts[i].setFillColor(sf::Color::White);

        std::ostringstream ss;
        ss << i + 1 << ". " << entry.playerName << " - " << entry.tiempo << " s";

        scoreTexts[i].setString(ss.str());
        scoreTexts[i].setPosition(50.f, (float)y);

        window.draw(scoreTexts[i]);

        y += 35;
        i++;
        if (i >= 10) break;
    }

    btnBack.draw(window);
}

GameScreen ScoreScreen::handleEvent(const sf::Event& e) {
    if (btnBack.isClicked(e)) {
        return GameScreen::MAIN_MENU;
    }

    return GameScreen::SCORES;
}
