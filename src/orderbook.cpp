#include <algorithm>
#include <deque>
#include <functional>
#include <iostream>
#include <map>
#include <vector>
#include <unordered_map>

using namespace std;
struct Order { int id; bool isBuy; int price; int qty; };
struct Trade { int buyId, sellId; int price, qty; };
struct Location { bool isBuy; int price; };

class OrderBook {
public:
    map<int, deque<Order>, greater<int>> bids; // best (highest) first
    map<int, deque<Order>> asks;                    // best (lowest) first
    bool cancel(int id);
    vector<Trade> addOrder(Order o);
    void print() const;
    std::unordered_map<int, Location> where;   // id -> (side, price)
};



vector<Trade> OrderBook::addOrder(Order o) {
    vector<Trade> trades;

    if (!o.isBuy) {
    while (o.qty > 0 && !bids.empty()) {
        auto level = bids.begin();                 // highest bid
        if (level->first < o.price) break;         // bid too low, stop

        Order& resting = level->second.front();
        if (resting.qty == 0) {              // cancelled earlier
        level->second.pop_front();
        if (level->second.empty()) bids.erase(level);
        continue;
        }
        int q = min(o.qty, resting.qty);

        trades.push_back({resting.id, o.id, level->first, q});  // resting is the buyer

        o.qty -= q;
        resting.qty -= q;
        if (resting.qty == 0){
            where.erase(resting.id);
            level->second.pop_front();
        } 
        if (level->second.empty()) bids.erase(level);
    }
    // after the loop: leftover sell rests on the ask side
    if (o.qty > 0) {
    asks[o.price].push_back(o);          // bids in the buy branch
    where[o.id] = {false, o.price};      // {true, ...} in the buy branch
}
}
    else{

    // Buy order: keep matching while we still want contracts and sellers exist.
    while (o.qty > 0 && !asks.empty()) {
        auto level = asks.begin();                 // cheapest price level
        if (level->first > o.price) break;         // too expensive, stop

    Order& resting = level->second.front();
    if (resting.qty == 0) {                 // cancelled earlier
    level->second.pop_front();
    if (level->second.empty()) asks.erase(level);   
    continue;
    }
        int q = min(o.qty, resting.qty);      // how much can trade

        trades.push_back({o.id, resting.id, level->first, q});

        o.qty -= q;
        resting.qty -= q;
    if (resting.qty == 0) {
    where.erase(resting.id);
    level->second.pop_front();
}
if (level->second.empty()) asks.erase(level);
    }
        // Anything unfilled becomes a resting bid.
    if (o.qty > 0) {
    bids[o.price].push_back(o);  
    where[o.id] = {true, o.price};
}
    }


    return trades;
}
bool OrderBook::cancel(int id) {
    auto it = where.find(id);
    if (it == where.end()) return false;       // not in the book

    Location loc = it->second;                 // copy it: we erase the entry below
    int p = loc.price;
    if (loc.isBuy) {
    auto level = bids.find(p);
    if (level != bids.end()) {
        for (Order& o : level->second) {
            if (o.id == id) {
                o.qty = 0;
                break;
            }
        }
    }
} else {
    auto level = asks.find(p);
    if (level != asks.end()) {
        for (Order& o : level->second) {
            if (o.id == id) {
                o.qty = 0;
                break;
            }
        }
    }
    }

    where.erase(id);
    return true;
}

void OrderBook::print() const {
    cout << "ASKS\n";
    for (auto it = asks.rbegin(); it != asks.rend(); ++it)
        for (const Order& o : it->second)
            cout << "  " << it->first << " x" << o.qty << " (id " << o.id << ")\n";
    cout << "BIDS\n";
    for (const auto& [price, dq] : bids)
        for (const Order& o : dq)
            cout << "  " << price << " x" << o.qty << " (id " << o.id << ")\n";
}


void printTrades(const vector<Trade>& trades) {
    if (trades.empty()) cout << "  (no trades)\n";
    for (const Trade& t : trades)
        cout << "  buy " << t.buyId << " / sell " << t.sellId
    << " : " << t.qty << " @ " << t.price << "\n";
}

int main() {
    OrderBook book;
    book.addOrder({1, false, 52, 3});
    book.addOrder({2, false, 52, 2});
    book.addOrder({3, false, 52, 4});

    cout << "--- start ---\n";
    book.print();

    cout << "\n--- cancel(2):  ---\n";
    cout << "  " << book.cancel(2) << "\n";
    book.print();

    cout << "\n--- buy {9, 52, qty 10}: ---\n";
    printTrades(book.addOrder({9, true, 52, 10}));
    book.print();

    cout << "\n--- cancel(2) again:  ---\n";
    cout << "  " << book.cancel(2) << "\n";

    cout << "\n--- cancel(1), already filled:  ---\n";
    cout << "  " << book.cancel(1) << "\n";

    cout << "\n--- cancel(9), resting bid:  ---\n";
    cout << "  " << book.cancel(9) << "\n";
    book.print();

    cout << "\n--- cancel(999), never existed:  ---\n";
    cout << "  " << book.cancel(999) << "\n";
}