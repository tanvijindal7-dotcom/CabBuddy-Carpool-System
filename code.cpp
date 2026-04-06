#include <iostream>
#include <unordered_map>
#include <queue>
#include <stack>
#include <list>
#include <vector>
#include <fstream>
#include <sstream>
#include <limits>
#include <algorithm>

using namespace std;

// ============================
//   STUDENT CLASS
// ============================
class Student {
public:
    string roll;
    string name;
    string email;

    Student() {}
    Student(const string &r, const string &n, const string &e)
        : roll(r), name(n), email(e) {}
};

// ============================
//   RIDE REQUEST CLASS
// ============================
class RideRequest {
public:
    string roll;
    string pickup;
    string destination;
    int time; // HHMM format

    RideRequest() {}
    RideRequest(const string &r, const string &p, const string &d, int t)
        : roll(r), pickup(p), destination(d), time(t) {}
};

// Sort earlier times first
class CompareTime {
public:
    bool operator()(const RideRequest &a, const RideRequest &b) const {
        return a.time > b.time;
    }
};

// ============================
//   CARPOOL GROUP (NO DRIVER)
// ============================
class CarpoolGroup {
public:
    int groupId;
    list<string> members;
    string pickup;
    string destination;
    int time;

    CarpoolGroup() : groupId(-1), time(0) {}

    CarpoolGroup(int id, const string &p, const string &d, int t)
        : groupId(id), pickup(p), destination(d), time(t) {}

    void addMember(const string &roll) {
        members.push_back(roll);
    }

    void display() const {
        cout << "---------------------------------\n";
        cout << "Carpool Group ID: " << groupId << "\n";
        cout << "Pickup: " << pickup << "  ->  Destination: " << destination << "\n";
        cout << "Time: " << time << " (HHMM)\n";
        cout << "Members: ";
        for (auto &m : members) cout << m << " ";
        cout << "\n---------------------------------\n";
    }
};

// ============================
//   ROUTE GRAPH
// ============================
class RouteGraph {
private:
    unordered_map<string, vector<pair<string, int>>> adj;

public:
    void addLocation(const string &loc) {
        if (!adj.count(loc)) adj[loc] = {};
    }

    void addEdge(const string &from, const string &to, int dist) {
        addLocation(from);
        addLocation(to);
        adj[from].push_back({to, dist});
        adj[to].push_back({from, dist});
    }

    void displayNeighbors(const string &loc) const {
        if (!adj.count(loc)) {
            cout << "Location not found.\n";
            return;
        }
        cout << "Neighbors of " << loc << ":\n";
        for (auto &x : adj.at(loc)) {
            cout << " -> " << x.first << " (" << x.second << " km)\n";
        }
    }

    void displayAll() const {
        cout << "\n======= Location Graph =======\n";
        for (auto &p : adj) {
            cout << p.first << " : ";
            for (auto &x : p.second)
                cout << "(" << x.first << ", " << x.second << "km) ";
            cout << "\n";
        }
        cout << "==============================\n";
    }
};

// ============================
//   MAIN APP CLASS
// ============================
class CabBuddyApp {
private:
    unordered_map<string, Student> students;
    queue<RideRequest> requestQueue;
    priority_queue<RideRequest, vector<RideRequest>, CompareTime> requestPQ;
    vector<CarpoolGroup> groups;

    stack<string> historyStack, redoStack;

    RouteGraph graph;

    string studentFile = "students.txt";
    string historyFile = "ride_history.txt";

    int nextGroupId = 1;

public:
    CabBuddyApp() {
        initGraph();
        loadStudentsFromFile();
        loadHistoryFromFile();
    }

    ~CabBuddyApp() {
        saveStudentsToFile();
        saveHistoryToFile();
    }

    // ============================================
    // Convert HHMM → minutes for accurate compare
    // ============================================
    int toMinutes(int hhmm) {
        int hh = hhmm / 100;
        int mm = hhmm % 100;
        return hh * 60 + mm;
    }

    // ============================================
    //   GRAPH (Your correct campus map)
    // ============================================
    void initGraph() {
        graph.addEdge("Sector62Campus", "Sector128Campus", 18);
        graph.addEdge("Sector62Campus", "BotanicalGardenMetro", 9);
        graph.addEdge("Sector128Campus", "OkhlaMetro", 6);
        graph.addEdge("Sector128Campus", "BotanicalGardenMetro", 7);
    }

    void clearInput() {
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
    }

    // ============================================
    // FILE HANDLING
    // ============================================
    void loadStudentsFromFile() {
        ifstream fin(studentFile);
        if (!fin.is_open()) return;

        string line;
        while (getline(fin, line)) {
            if (line.empty()) continue;

            string roll, name, email;
            stringstream ss(line);
            getline(ss, roll, '|');
            getline(ss, name, '|');
            getline(ss, email, '|');

            students[roll] = Student(roll, name, email);
        }
        fin.close();
    }

    void saveStudentsToFile() {
        ofstream fout(studentFile);
        for (auto &p : students)
            fout << p.second.roll << "|" << p.second.name << "|" << p.second.email << "|\n";
        fout.close();
    }

    void loadHistoryFromFile() {
        ifstream fin(historyFile);
        if (!fin.is_open()) return;

        string line;
        while (getline(fin, line))
            if (!line.empty()) historyStack.push(line);

        fin.close();
    }

    void saveHistoryToFile() {
        stack<string> temp = historyStack;
        vector<string> lines;
        while (!temp.empty()) {
            lines.push_back(temp.top());
            temp.pop();
        }
        reverse(lines.begin(), lines.end());

        ofstream fout(historyFile);
        for (auto &x : lines) fout << x << "\n";
        fout.close();
    }

    // ============================================
    // STUDENT SYSTEM
    // ============================================
    void registerStudent() {
        string roll, name, email;

        cout << "Enter Roll No: ";
        cin >> roll;
        clearInput();

        cout << "Enter Name: ";
        getline(cin, name);

        cout << "Enter Email: ";
        getline(cin, email);

        if (students.count(roll)) {
            cout << "Already registered.\n";
            return;
        }

        students[roll] = Student(roll, name, email);
        cout << "Student Registered!\n";

        historyStack.push("Registered student: " + roll);
        while (!redoStack.empty()) redoStack.pop();
    }

    void listStudents() {
        cout << "\n===== Students =====\n";
        if (students.empty()) {
            cout << "No students registered.\n";
            return;
        }

        for (auto &p : students)
            cout << p.second.roll << " - " << p.second.name
                 << " - " << p.second.email << "\n";
    }

    bool isStudentRegistered(const string &roll) {
        return students.count(roll);
    }

    // ============================================
    // RIDE REQUESTS
    // ============================================
    void addRideRequest() {
        string roll, pickup, dest;
        int time;

        cout << "Enter Roll No: ";
        cin >> roll;
        clearInput();

        if (!isStudentRegistered(roll)) {
            cout << "Not registered.\n";
            return;
        }

        cout << "Pickup: ";
        getline(cin, pickup);

        cout << "Destination: ";
        getline(cin, dest);

        cout << "Time (HHMM): ";
        cin >> time;
        clearInput();

        RideRequest req(roll, pickup, dest, time);

        requestQueue.push(req);
        requestPQ.push(req);

        cout << "Ride Request Added!\n";

        historyStack.push("Ride request by " + roll);
        while (!redoStack.empty()) redoStack.pop();
    }

    void showPendingRequests() {
        cout << "\n===== Pending Requests =====\n";
        if (requestQueue.empty()) {
            cout << "No pending requests.\n";
            return;
        }

        queue<RideRequest> temp = requestQueue;
        while (!temp.empty()) {
            auto r = temp.front();
            temp.pop();
            cout << r.roll << " | " << r.pickup << " -> "
                 << r.destination << " at " << r.time << "\n";
        }
    }

    void removeFromQueue(const RideRequest &req) {
        queue<RideRequest> tmp;

        while (!requestQueue.empty()) {
            RideRequest r = requestQueue.front();
            requestQueue.pop();

            if (!(r.roll == req.roll &&
                  r.pickup == req.pickup &&
                  r.destination == req.destination &&
                  r.time == req.time)) {
                tmp.push(r);
            }
        }
        requestQueue = tmp;
    }

    // ============================================
    // GROUP FORMATION — Pickup + Dest same, Time ±15 min
    // ============================================
    void autoMatchRides() {
        if (requestPQ.size() < 2) {
            cout << "Not enough requests.\n";
            return;
        }

        RideRequest first = requestPQ.top();
        requestPQ.pop();

        vector<RideRequest> buffer;
        bool found = false;
        RideRequest second;

        while (!requestPQ.empty()) {
            RideRequest r = requestPQ.top();
            requestPQ.pop();

            if (r.pickup == first.pickup &&
                r.destination == first.destination &&
                abs(toMinutes(r.time) - toMinutes(first.time)) <= 15) {

                second = r;
                found = true;
                break;
            } else {
                buffer.push_back(r);
            }
        }

        for (auto &x : buffer) requestPQ.push(x);

        if (!found) {
            requestPQ.push(first);
            cout << "No matching ride found.\n";
            return;
        }

        removeFromQueue(first);
        removeFromQueue(second);

        CarpoolGroup group(nextGroupId++, first.pickup, first.destination, first.time);
        group.addMember(first.roll);
        group.addMember(second.roll);

        addMoreMatches(group);

        groups.push_back(group);

        cout << "Carpool Group Formed!\n";
        group.display();

        historyStack.push("Group created: " + to_string(group.groupId));
        while (!redoStack.empty()) redoStack.pop();
    }

    void addMoreMatches(CarpoolGroup &grp) {
        vector<RideRequest> temp;

        while (!requestPQ.empty()) {
            RideRequest r = requestPQ.top();
            requestPQ.pop();

            if (r.pickup == grp.pickup &&
                r.destination == grp.destination &&
                abs(toMinutes(r.time) - toMinutes(grp.time)) <= 15) {

                grp.addMember(r.roll);
                removeFromQueue(r);
            } else {
                temp.push_back(r);
            }
        }

        for (auto &x : temp) requestPQ.push(x);
    }

    // ============================================
    // GROUP DISPLAY
    // ============================================
    void showGroups() {
        cout << "\n===== Carpool Groups =====\n";
        if (groups.empty()) {
            cout << "No groups yet.\n";
            return;
        }

        for (auto &g : groups) g.display();
    }

    // ============================================
    // HISTORY SYSTEM
    // ============================================
    void showHistory() {
        cout << "\n===== History =====\n";

        if (historyStack.empty()) {
            cout << "No history.\n";
            return;
        }

        stack<string> temp = historyStack;
        while (!temp.empty()) {
            cout << temp.top() << "\n";
            temp.pop();
        }
    }

    void undo() {
        if (historyStack.empty()) {
            cout << "Nothing to undo.\n";
            return;
        }

        string last = historyStack.top();
        historyStack.pop();
        redoStack.push(last);

        cout << "Undone: " << last << "\n";
    }

    void redo() {
        if (redoStack.empty()) {
            cout << "Nothing to redo.\n";
            return;
        }

        string last = redoStack.top();
        redoStack.pop();
        historyStack.push(last);

        cout << "Redone: " << last << "\n";
    }

    // ============================================
    // GRAPH VIEW
    // ============================================
    void showGraph() { graph.displayAll(); }
    void showNeighbors() {
        string loc;
        cout << "Enter location: ";
        cin >> loc;
        graph.displayNeighbors(loc);
    }

    // ============================================
    // MENU
    // ============================================
    void showMenu() {
        cout << "\n========== CabBuddy Menu ==========\n";
        cout << "1. Register Student\n";
        cout << "2. List Students\n";
        cout << "3. Add Ride Request\n";
        cout << "4. Show Pending Requests\n";
        cout << "5. Auto Match Rides\n";
        cout << "6. Show Carpool Groups\n";
        cout << "7. Show History\n";
        cout << "8. Undo Last Action\n";
        cout << "9. Redo Last Action\n";
        cout << "10. Show Location Graph\n";
        cout << "11. Show Neighbors\n";
        cout << "0. Exit\n";
        cout << "Enter choice: ";
    }

    void run() {
        int ch;

        while (true) {
            showMenu();

            if (!(cin >> ch)) {
                clearInput();
                cout << "Invalid input.\n";
                continue;
            }
            clearInput();

            switch (ch) {
                case 1: registerStudent(); break;
                case 2: listStudents(); break;
                case 3: addRideRequest(); break;
                case 4: showPendingRequests(); break;
                case 5: autoMatchRides(); break;
                case 6: showGroups(); break;
                case 7: showHistory(); break;
                case 8: undo(); break;
                case 9: redo(); break;
                case 10: showGraph(); break;
                case 11: showNeighbors(); break;
                case 0:
                    cout << "Exiting CabBuddy...\n";
                    return;
                default:
                    cout << "Invalid choice.\n";
            }
        }
    }
};

// ============================
// MAIN FUNCTION
// ============================
int main() {

    cout << "=====================================\n";
    cout << "      CabBuddy - Campus Carpool      \n";
    cout << "=====================================\n";

    CabBuddyApp app;
    app.run();

    return 0;
}
