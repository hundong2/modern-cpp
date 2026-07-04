#include <iostream>
#include <numeric>
#include <string>
#include <vector>

struct LineItem {
    std::string name;
    int unit_price_won{};
    int quantity{};
};

struct Order {
    std::vector<LineItem> items;

    int total() const {
        return std::accumulate(items.begin(), items.end(), 0,
            [](int sum, const LineItem& item) {
                return sum + item.unit_price_won * item.quantity;
            });
    }
};

class PaymentGateway {
public:
    virtual ~PaymentGateway() = default;
    virtual bool pay(int amount_won) = 0;
};

class ConsolePaymentGateway final : public PaymentGateway {
public:
    bool pay(int amount_won) override {
        std::cout << "[payment] " << amount_won << " won approved\n";
        return true;
    }
};

class ReceiptPrinter {
public:
    void print(const Order& order) const {
        std::cout << "Receipt\n";
        for (const auto& item : order.items) {
            std::cout << "- " << item.name << " x " << item.quantity
                      << " = " << item.unit_price_won * item.quantity << " won\n";
        }
        std::cout << "Total: " << order.total() << " won\n";
    }
};

class OrderService {
public:
    OrderService(PaymentGateway& payment_gateway, ReceiptPrinter& receipt_printer)
        : payment_gateway_{payment_gateway}, receipt_printer_{receipt_printer} {}

    void checkout(const Order& order) {
        const int amount = order.total();

        if (payment_gateway_.pay(amount)) {
            receipt_printer_.print(order);
        }
    }

private:
    // 참조 멤버는 객체를 소유하지 않습니다.
    // 이 서비스보다 payment_gateway_와 receipt_printer_가 오래 살아 있어야 합니다.
    PaymentGateway& payment_gateway_;
    ReceiptPrinter& receipt_printer_;
};

int main() {
    Order order{{
        {"book", 25000, 1},
        {"pen", 1500, 3},
    }};

    ConsolePaymentGateway payment_gateway;
    ReceiptPrinter receipt_printer;
    OrderService service{payment_gateway, receipt_printer};

    service.checkout(order);
}

