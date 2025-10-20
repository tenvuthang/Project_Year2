#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <limits>
#include <numeric>
#include <sstream>
#include <string>
#include <vector>

namespace
{
    constexpr const char* COLOR_RESET = "\x1b[0m";
    constexpr const char* COLOR_GREEN = "\x1b[32m";
    constexpr const char* COLOR_YELLOW = "\x1b[33m";
    constexpr const char* COLOR_RED = "\x1b[31m";

    std::string trim(const std::string& text)
    {
        const auto first = text.find_first_not_of(" \t\r\n");
        if (first == std::string::npos)
        {
            return {};
        }
        const auto last = text.find_last_not_of(" \t\r\n");
        return text.substr(first, last - first + 1);
    }

    std::string toLower(const std::string& value)
    {
        std::string lowered;
        lowered.reserve(value.size());
        for (char ch : value)
        {
            lowered.push_back(static_cast<char>(std::tolower(static_cast<unsigned char>(ch))));
        }
        return lowered;
    }

    std::string repeat(const std::string& text, int count)
    {
        std::string result;
        const int safeCount = std::max(count, 0);
        result.reserve(text.size() * static_cast<std::size_t>(safeCount));
        for (int i = 0; i < safeCount; ++i)
        {
            result += text;
        }
        return result;
    }

    void clearScreen()
    {
#ifdef _WIN32
        std::system("cls");
#else
        std::system("clear");
#endif
    }

    void waitForEnter()
    {
        std::cout << "\nNhấn Enter để quay lại menu...";
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    }

    int readInt(const std::string& prompt, int minValue, int maxValue)
    {
        while (true)
        {
            std::cout << prompt;
            std::string line;
            if (!std::getline(std::cin, line))
            {
                return minValue;
            }

            std::stringstream ss(line);
            int value = 0;
            if (!(ss >> value))
            {
                std::cout << "Giá trị không hợp lệ, vui lòng thử lại.\n";
                continue;
            }

            if (value < minValue || value > maxValue)
            {
                std::cout << "Vui lòng nhập số trong khoảng [" << minValue << ", " << maxValue << "].\n";
                continue;
            }

            return value;
        }
    }

    int readPositiveInt(const std::string& prompt)
    {
        while (true)
        {
            std::cout << prompt;
            std::string line;
            if (!std::getline(std::cin, line))
            {
                return 0;
            }

            std::stringstream ss(line);
            int value = 0;
            if (!(ss >> value) || value < 0)
            {
                std::cout << "Giá trị không hợp lệ, vui lòng thử lại.\n";
                continue;
            }

            return value;
        }
    }

    std::string buildSeparator(const std::string& left,
                               const std::string& mid,
                               const std::string& right,
                               const std::vector<int>& widths)
    {
        std::ostringstream oss;
        oss << left;
        for (std::size_t i = 0; i < widths.size(); ++i)
        {
            oss << repeat("─", widths[i]);
            if (i + 1 < widths.size())
            {
                oss << mid;
            }
        }
        oss << right;
        return oss.str();
    }
} // namespace

class Team
{
public:
    explicit Team(std::string name = "") : name(std::move(name)) {}

    void updateMatch(int scored, int conceded)
    {
        played += 1;
        goalsFor += scored;
        goalsAgainst += conceded;

        if (scored > conceded)
        {
            wins += 1;
            points += 3;
        }
        else if (scored == conceded)
        {
            draws += 1;
            points += 1;
        }
        else
        {
            losses += 1;
        }
    }

    void setStats(int playedValue,
                  int winsValue,
                  int drawsValue,
                  int lossesValue,
                  int goalsForValue,
                  int goalsAgainstValue,
                  int pointsValue)
    {
        played = playedValue;
        wins = winsValue;
        draws = drawsValue;
        losses = lossesValue;
        goalsFor = goalsForValue;
        goalsAgainst = goalsAgainstValue;
        points = pointsValue;
    }

    const std::string& getName() const { return name; }
    int getPlayed() const { return played; }
    int getWins() const { return wins; }
    int getDraws() const { return draws; }
    int getLosses() const { return losses; }
    int getGoalsFor() const { return goalsFor; }
    int getGoalsAgainst() const { return goalsAgainst; }
    int getPoints() const { return points; }

    int goalDifference() const { return goalsFor - goalsAgainst; }

    bool operator<(const Team& other) const
    {
        if (points != other.points)
        {
            return points > other.points;
        }
        if (goalDifference() != other.goalDifference())
        {
            return goalDifference() > other.goalDifference();
        }
        if (goalsFor != other.goalsFor)
        {
            return goalsFor > other.goalsFor;
        }
        return toLower(name) < toLower(other.name);
    }

    friend std::ostream& operator<<(std::ostream& os, const Team& team)
    {
        os << team.name << " | "
           << "Pts: " << team.points << " | "
           << "W-D-L: " << team.wins << "-" << team.draws << "-" << team.losses << " | "
           << "GF/GA: " << team.goalsFor << "/" << team.goalsAgainst;
        return os;
    }

private:
    std::string name;
    int played = 0;
    int wins = 0;
    int draws = 0;
    int losses = 0;
    int goalsFor = 0;
    int goalsAgainst = 0;
    int points = 0;
};

struct TopScorer
{
    std::string playerName;
    std::string teamName;
    int goals = 0;
};

class League
{
public:
    void addTeam(const Team& team)
    {
        if (Team* existing = findTeam(team.getName()))
        {
            *existing = team;
        }
        else
        {
            teams.push_back(team);
        }
    }

    bool loadTeamsFromFile(const std::string& path)
    {
        std::ifstream input(path);
        if (!input)
        {
            return false;
        }

        std::vector<Team> loadedTeams;
        std::string line;
        while (std::getline(input, line))
        {
            line = trim(line);
            if (line.empty() || line.front() == '#')
            {
                continue;
            }

            std::vector<std::string> tokens;
            std::stringstream ss(line);
            std::string token;
            while (std::getline(ss, token, ';'))
            {
                tokens.push_back(trim(token));
            }

            if (tokens.empty())
            {
                continue;
            }

            if (tokens.size() == 1)
            {
                const std::string& maybeCount = tokens.front();
                const bool numericLine = !maybeCount.empty()
                                         && std::all_of(maybeCount.begin(),
                                                        maybeCount.end(),
                                                        [](unsigned char ch) {
                                                            return std::isdigit(static_cast<unsigned char>(ch));
                                                        });
                if (numericLine)
                {
                    continue;
                }
            }

            Team team(tokens.front());
            if (tokens.size() >= 6)
            {
                try
                {
                    const int wins = std::stoi(tokens[1]);
                    const int draws = std::stoi(tokens[2]);
                    const int losses = std::stoi(tokens[3]);
                    const int goalsFor = std::stoi(tokens[4]);
                    const int goalsAgainst = std::stoi(tokens[5]);
                    const int points = (tokens.size() >= 7) ? std::stoi(tokens[6]) : (wins * 3 + draws);
                    const int played = wins + draws + losses;
                    team.setStats(played, wins, draws, losses, goalsFor, goalsAgainst, points);
                }
                catch (...)
                {
                    // Ignore malformed line.
                    continue;
                }
            }
            loadedTeams.push_back(team);

            if (loadedTeams.size() >= 20)
            {
                break;
            }
        }

        if (loadedTeams.empty())
        {
            return false;
        }

        teams = std::move(loadedTeams);
        return true;
    }

    bool saveRankingToText(const std::string& path) const
    {
        std::ofstream output(path);
        if (!output)
        {
            return false;
        }
        printTable(output, false);
        return true;
    }

    bool updateMatch(const std::string& homeName, const std::string& awayName, int homeGoals, int awayGoals)
    {
        Team* homeTeam = findTeam(homeName);
        Team* awayTeam = findTeam(awayName);

        if (!homeTeam || !awayTeam)
        {
            return false;
        }

        homeTeam->updateMatch(homeGoals, awayGoals);
        awayTeam->updateMatch(awayGoals, homeGoals);
        return true;
    }

    void addOrUpdateTopScorer(const std::string& playerName, const std::string& teamName, int goals)
    {
        auto it = std::find_if(topScorers.begin(), topScorers.end(), [&](const TopScorer& scorer) {
            return toLower(scorer.playerName) == toLower(playerName);
        });

        if (it != topScorers.end())
        {
            it->goals += goals;
            if (!teamName.empty())
            {
                it->teamName = teamName;
            }
        }
        else
        {
            topScorers.push_back({playerName, teamName, goals});
        }

        std::sort(topScorers.begin(), topScorers.end(), [](const TopScorer& a, const TopScorer& b) {
            if (a.goals != b.goals)
            {
                return a.goals > b.goals;
            }
            return toLower(a.playerName) < toLower(b.playerName);
        });
    }

    bool loadTopScorersFromFile(const std::string& path)
    {
        std::ifstream input(path);
        if (!input)
        {
            return false;
        }

        std::vector<TopScorer> loaded;
        std::string line;
        while (std::getline(input, line))
        {
            line = trim(line);
            if (line.empty() || line.front() == '#')
            {
                continue;
            }

            std::vector<std::string> tokens;
            std::stringstream ss(line);
            std::string token;
            while (std::getline(ss, token, ';'))
            {
                tokens.push_back(trim(token));
            }

            if (tokens.size() < 3)
            {
                continue;
            }

            try
            {
                const int goals = std::stoi(tokens[2]);
                loaded.push_back({tokens[0], tokens[1], goals});
            }
            catch (...)
            {
                continue;
            }
        }

        if (loaded.empty())
        {
            return false;
        }

        std::sort(loaded.begin(), loaded.end(), [](const TopScorer& a, const TopScorer& b) {
            if (a.goals != b.goals)
            {
                return a.goals > b.goals;
            }
            if (toLower(a.playerName) != toLower(b.playerName))
            {
                return toLower(a.playerName) < toLower(b.playerName);
            }
            return toLower(a.teamName) < toLower(b.teamName);
        });

        topScorers = std::move(loaded);
        return true;
    }

    void printTopScorers(std::ostream& os) const
    {
        if (topScorers.empty())
        {
            os << "Chưa có dữ liệu vua phá lưới.\n";
            return;
        }

        const auto previousFlags = os.flags();

        const std::vector<int> widths = {4, 25, 22, 10};

        os << std::left << std::setw(widths[0]) << "No"
           << std::setw(widths[1]) << "Cầu thủ"
           << std::setw(widths[2]) << "Đội"
           << std::right << std::setw(widths[3]) << "Bàn thắng" << '\n';

        os << std::string(61, '-') << '\n';

        for (std::size_t i = 0; i < topScorers.size(); ++i)
        {
            const TopScorer& scorer = topScorers[i];
            os << std::left << std::setw(widths[0]) << (i + 1)
               << std::setw(widths[1]) << scorer.playerName
               << std::setw(widths[2]) << scorer.teamName
               << std::right << std::setw(widths[3]) << scorer.goals << '\n';
        }

        os.flags(previousFlags);
    }

    void printTable(std::ostream& os, bool useColor) const
    {
        if (teams.empty())
        {
            os << "Chưa có dữ liệu đội bóng.\n";
            return;
        }

        const auto sorted = getSortedTable();
        const std::vector<int> widths = {4, 20, 4, 4, 4, 4, 5, 5, 5, 5};

        const std::string topLine = buildSeparator("┌", "┬", "┐", widths);
        const std::string midLine = buildSeparator("├", "┼", "┤", widths);
        const std::string bottomLine = buildSeparator("└", "┴", "┘", widths);

        os << topLine << '\n';
        {
            std::ostringstream header;
            header << "│" << std::setw(widths[0]) << std::right << "Pos";
            header << "│" << std::setw(widths[1]) << std::left << "Team";
            header << std::right;
            header << "│" << std::setw(widths[2]) << std::right << "MP";
            header << "│" << std::setw(widths[3]) << std::right << "W";
            header << "│" << std::setw(widths[4]) << std::right << "D";
            header << "│" << std::setw(widths[5]) << std::right << "L";
            header << "│" << std::setw(widths[6]) << std::right << "GF";
            header << "│" << std::setw(widths[7]) << std::right << "GA";
            header << "│" << std::setw(widths[8]) << std::right << "GD";
            header << "│" << std::setw(widths[9]) << std::right << "Pts";
            header << "│";
            os << header.str() << '\n';
        }
        os << midLine << '\n';

        const bool colorize = useColor && (&os == &std::cout);
        for (std::size_t i = 0; i < sorted.size(); ++i)
        {
            const Team& team = sorted[i];
            std::ostringstream row;

            row << "│" << std::setw(widths[0]) << std::right << (i + 1);
            row << "│" << std::setw(widths[1]) << std::left << team.getName();
            row << std::right;
            row << "│" << std::setw(widths[2]) << std::right << team.getPlayed();
            row << "│" << std::setw(widths[3]) << std::right << team.getWins();
            row << "│" << std::setw(widths[4]) << std::right << team.getDraws();
            row << "│" << std::setw(widths[5]) << std::right << team.getLosses();
            row << "│" << std::setw(widths[6]) << std::right << team.getGoalsFor();
            row << "│" << std::setw(widths[7]) << std::right << team.getGoalsAgainst();
            row << "│" << std::setw(widths[8]) << std::right << team.goalDifference();
            row << "│" << std::setw(widths[9]) << std::right << team.getPoints() << "│";

            if (colorize)
            {
                const std::string color = colorForPosition(i, sorted.size());
                if (!color.empty())
                {
                    os << color;
                }
                os << row.str();
                if (!color.empty())
                {
                    os << COLOR_RESET;
                }
                os << '\n';
            }
            else
            {
                os << row.str() << '\n';
            }
        }

        os << bottomLine << '\n';
    }

    void printAggregateStats(std::ostream& os) const
    {
        if (teams.empty())
        {
            os << "Chưa có dữ liệu để thống kê.\n";
            return;
        }

        const int totalGoals = std::accumulate(teams.begin(), teams.end(), 0, [](int acc, const Team& team) {
            return acc + team.getGoalsFor();
        });

        const int totalMatchesCountedTwice = std::accumulate(teams.begin(), teams.end(), 0, [](int acc, const Team& team) {
            return acc + team.getPlayed();
        });

        const int totalMatches = totalMatchesCountedTwice / 2;

        double averageGoalsPerMatch = 0.0;
        if (totalMatches > 0)
        {
            averageGoalsPerMatch = static_cast<double>(totalGoals) / static_cast<double>(totalMatches);
        }

        const auto topScoring = std::max_element(teams.begin(), teams.end(), [](const Team& a, const Team& b) {
            return a.getGoalsFor() < b.getGoalsFor();
        });
        const auto bestDefense = std::min_element(teams.begin(), teams.end(), [](const Team& a, const Team& b) {
            return a.getGoalsAgainst() < b.getGoalsAgainst();
        });

        os << "Tổng số bàn thắng: " << totalGoals << '\n';
        os << "Tổng số trận (ước lượng): " << totalMatches << '\n';
        os << "Trung bình bàn/trận: " << std::fixed << std::setprecision(2) << averageGoalsPerMatch << '\n';
        os << std::defaultfloat;

        if (topScoring != teams.end())
        {
            os << "Đội ghi nhiều bàn nhất: " << topScoring->getName() << " (" << topScoring->getGoalsFor() << " bàn)\n";
        }
        if (bestDefense != teams.end())
        {
            os << "Đội phòng ngự chắc nhất: " << bestDefense->getName() << " (" << bestDefense->getGoalsAgainst() << " bàn thua)\n";
        }
    }

    void filterByPoints(std::ostream& os, int threshold) const
    {
        const auto sorted = getSortedTable();
        os << "Các đội có điểm > " << threshold << ":\n";
        bool found = false;
        for (const Team& team : sorted)
        {
            if (team.getPoints() > threshold)
            {
                os << "- " << team << '\n';
                found = true;
            }
        }
        if (!found)
        {
            os << "Không có đội nào đạt yêu cầu.\n";
        }
    }

    void showTopTeams(std::ostream& os, int count) const
    {
        const auto sorted = getSortedTable();
        os << "Top " << count << " đội dẫn đầu:\n";
        for (int i = 0; i < count && i < static_cast<int>(sorted.size()); ++i)
        {
            os << (i + 1) << ". " << sorted[i] << '\n';
        }
    }

    void showRelegationZone(std::ostream& os, int count) const
    {
        const auto sorted = getSortedTable();
        if (sorted.empty())
        {
            os << "Chưa có dữ liệu.\n";
            return;
        }
        os << "Nhóm xuống hạng (" << count << " đội cuối bảng):\n";
        const int total = static_cast<int>(sorted.size());
        for (int i = total - count; i < total; ++i)
        {
            if (i >= 0)
            {
                os << (i + 1) << ". " << sorted[static_cast<std::size_t>(i)] << '\n';
            }
        }
    }

    bool empty() const { return teams.empty(); }

private:
    Team* findTeam(const std::string& name)
    {
        const std::string target = toLower(name);
        for (Team& team : teams)
        {
            if (toLower(team.getName()) == target)
            {
                return &team;
            }
        }
        return nullptr;
    }

    const Team* findTeam(const std::string& name) const
    {
        const std::string target = toLower(name);
        for (const Team& team : teams)
        {
            if (toLower(team.getName()) == target)
            {
                return &team;
            }
        }
        return nullptr;
    }

    std::vector<Team> getSortedTable() const
    {
        std::vector<Team> sorted = teams;
        std::sort(sorted.begin(), sorted.end());
        return sorted;
    }

    std::string colorForPosition(std::size_t index, std::size_t total) const
    {
        if (index < 4)
        {
            return COLOR_GREEN;
        }
        if (index < 6)
        {
            return COLOR_YELLOW;
        }
        if (index >= total - 3)
        {
            return COLOR_RED;
        }
        return {};
    }

    std::vector<Team> teams;
    std::vector<TopScorer> topScorers;
};

int main()
{
    League league;
    bool running = true;

    while (running)
    {
        clearScreen();
        std::cout << "===== QUẢN LÝ BẢNG XẾP HẠNG EPL =====\n";
        std::cout << "1. Nạp danh sách đội từ file\n";
        std::cout << "2. Hiển thị bảng xếp hạng\n";
        std::cout << "3. Cập nhật kết quả trận đấu\n";
        std::cout << "4. Thống kê tổng hợp\n";
        std::cout << "5. Truy vấn & lọc đội bóng\n";
        std::cout << "6. Xuất bảng xếp hạng ra ranking.txt\n";
        std::cout << "7. Quản lý vua phá lưới\n";
        std::cout << "0. Thoát\n";

        const int choice = readInt("Chọn chức năng: ", 0, 7);

        switch (choice)
        {
        case 1:
        {
            std::cout << "Nhập đường dẫn file (Enter để dùng teams.txt): ";
            std::string path;
            std::getline(std::cin, path);
            if (path.empty())
            {
                path = "teams.txt";
            }
            if (league.loadTeamsFromFile(path))
            {
                std::cout << "Đã nạp dữ liệu từ \"" << path << "\".\n";
            }
            else
            {
                std::cout << "Không thể nạp dữ liệu. Vui lòng kiểm tra file.\n";
            }
            waitForEnter();
            break;
        }
        case 2:
        {
            league.printTable(std::cout, true);
            waitForEnter();
            break;
        }
        case 3:
        {
            if (league.empty())
            {
                std::cout << "Vui lòng nạp danh sách đội trước.\n";
                waitForEnter();
                break;
            }

            std::string home, away;
            std::cout << "Tên đội chủ nhà: ";
            std::getline(std::cin, home);
            std::cout << "Tên đội khách: ";
            std::getline(std::cin, away);

            const int homeGoals = readInt("Bàn thắng đội chủ nhà: ", 0, 20);
            const int awayGoals = readInt("Bàn thắng đội khách: ", 0, 20);

            if (league.updateMatch(home, away, homeGoals, awayGoals))
            {
                std::cout << "Đã cập nhật trận đấu " << home << " " << homeGoals << " - "
                          << awayGoals << " " << away << ".\n";
            }
            else
            {
                std::cout << "Không tìm thấy đội. Vui lòng kiểm tra tên.\n";
            }
            waitForEnter();
            break;
        }
        case 4:
        {
            league.printAggregateStats(std::cout);
            waitForEnter();
            break;
        }
        case 5:
        {
            if (league.empty())
            {
                std::cout << "Vui lòng nạp danh sách đội trước.\n";
                waitForEnter();
                break;
            }

            std::cout << "1. Liệt kê đội có điểm > X\n";
            std::cout << "2. Xem top 4 đội\n";
            std::cout << "3. Xem nhóm xuống hạng\n";
            const int filterChoice = readInt("Chọn: ", 1, 3);

            if (filterChoice == 1)
            {
                const int threshold = readPositiveInt("Nhập điểm X: ");
                league.filterByPoints(std::cout, threshold);
            }
            else if (filterChoice == 2)
            {
                league.showTopTeams(std::cout, 4);
            }
            else
            {
                league.showRelegationZone(std::cout, 3);
            }
            waitForEnter();
            break;
        }
        case 6:
        {
            if (league.empty())
            {
                std::cout << "Vui lòng nạp danh sách đội trước.\n";
                waitForEnter();
                break;
            }

            std::string path = "ranking.txt";
            if (league.saveRankingToText(path))
            {
                std::cout << "Đã ghi bảng xếp hạng ra \"" << path << "\".\n";
            }
            else
            {
                std::cout << "Không thể ghi file \"" << path << "\".\n";
            }
            waitForEnter();
            break;
        }
        case 7:
        {
            std::cout << "1. Xem danh sách vua phá lưới\n";
            std::cout << "2. Nạp danh sách từ file\n";
            const int scorerChoice = readInt("Chọn: ", 1, 2);
            if (scorerChoice == 1)
            {
                league.printTopScorers(std::cout);
            }
            else
            {
                std::cout << "Nhập đường dẫn file (Enter để dùng player.txt): ";
                std::string path;
                std::getline(std::cin, path);
                path = trim(path);
                if (path.empty())
                {
                    path = "player.txt";
                }

                if (league.loadTopScorersFromFile(path))
                {
                    std::cout << "Đã nạp vua phá lưới từ \"" << path << "\".\n";
                }
                else
                {
                    std::cout << "Không thể nạp dữ liệu từ \"" << path << "\".\n";
                }
            }
            waitForEnter();
            break;
        }
        case 0:
        default:
            running = false;
            break;
        }
    }

    std::cout << "Tạm biệt!\n";
    return 0;
}
