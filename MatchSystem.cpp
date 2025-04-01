#include <iostream>
#include <vector>
#include <algorithm>
#include <cmath>
#include <unordered_map>

struct Player {
    std::string steamID;
    int elo;
    int hoursPlayed;
    bool matched = false;
};

class Matchmaker {
private:
    std::vector<Player> players;
    const int MAX_ELO_DIFF = 150;
    const int MAX_HOURS_DIFF = 200;
    const int TEAM_SIZE = 5;

    float calculateMatchScore(const Player& p1, const Player& p2) {
        float eloDiff = std::abs(p1.elo - p2.elo) / (float)MAX_ELO_DIFF;
        float hoursDiff = std::abs(p1.hoursPlayed - p2.hoursPlayed) / (float)MAX_HOURS_DIFF;
        return (0.7f * eloDiff) + (0.3f * hoursDiff);
    }

public:
    void addPlayer(const Player& player) {
        players.push_back(player);
    }

    std::unordered_map<std::string, std::vector<Player>> createMatches() {
        std::unordered_map<std::string, std::vector<Player>> matches;
        int matchCount = 0;

        std::sort(players.begin(), players.end(), [](const Player& a, const Player& b) {
            return a.elo > b.elo;
        });

        for (size_t i = 0; i < players.size(); ++i) {
            if (players[i].matched) continue;

            std::vector<Player> team;
            team.push_back(players[i]);
            players[i].matched = true;

            for (int j = 0; j < TEAM_SIZE - 1; ++j) {
                float bestScore = INFINITY;
                int bestIndex = -1;

                for (size_t k = i + 1; k < players.size(); ++k) {
                    if (players[k].matched) continue;

                    float currentScore = 0;
                    for (const auto& member : team) {
                        currentScore += calculateMatchScore(member, players[k]);
                    }

                    if (currentScore < bestScore) {
                        bestScore = currentScore;
                        bestIndex = k;
                    }
                }

                if (bestIndex != -1) {
                    team.push_back(players[bestIndex]);
                    players[bestIndex].matched = true;
                }
            }

            if (team.size() == TEAM_SIZE) {
                matches["Match_" + std::to_string(++matchCount)] = team;
            }
        }

        return matches;
    }
};

int main() {
    std::vector<Player> players = {
        {"STEAM_1:1:12345", 1520, 856},
        {"STEAM_1:1:54321", 1680, 1200},
        {"STEAM_1:1:98765", 1420, 650},
        {"STEAM_1:1:13579", 1580, 950},
        {"STEAM_1:1:24680", 1460, 720},
        {"STEAM_1:1:11223", 1620, 1100},
        {"STEAM_1:1:33445", 1380, 580},
        {"STEAM_1:1:55667", 1490, 820},
        {"STEAM_1:1:77889", 1550, 920},
        {"STEAM_1:1:99000", 1440, 680}
    };

    Matchmaker matchmaker;
    for (const auto& player : players) {
        matchmaker.addPlayer(player);
    }

    auto matches = matchmaker.createMatches();

    for (const auto& [matchId, team] : matches) {
        std::cout << "\n" << matchId << ":\n";
        std::cout << "-------------------------------------------------\n";
        std::cout << "| SteamID          | ELO  | Hours Played |\n";
        std::cout << "-------------------------------------------------\n";
        
        int avgElo = 0;
        int avgHours = 0;
        
        for (const auto& player : team) {
            std::printf("| %-16s | %-4d | %-12d |\n", 
                        player.steamID.c_str(), 
                        player.elo, 
                        player.hoursPlayed);
            avgElo += player.elo;
            avgHours += player.hoursPlayed;
        }
        
        avgElo /= TEAM_SIZE;
        avgHours /= TEAM_SIZE;
        
        std::cout << "-------------------------------------------------\n";
        std::cout << "| Avg:            | " << avgElo << "  | " << avgHours << "        |\n";
        std::cout << "-------------------------------------------------\n";
    }

    return 0;
}
