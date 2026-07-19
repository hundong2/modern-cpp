#include <iostream>
#include <memory>
#include <optional>
#include <string>
#include <utility>
#include <vector>

class Account {
public:
    Account(std::string owner, int balance)
        : owner_(std::move(owner)), balance_(balance)
    {
    }

    void deposit(int amount)
    {
        if (amount > 0) {
            balance_ += amount;
        }
    }

    bool withdraw(int amount)
    {
        if (amount <= 0 || amount > balance_) {
            return false;
        }
        balance_ -= amount;
        return true;
    }

    const std::string& owner() const
    {
        return owner_;
    }

    int balance() const
    {
        return balance_;
    }

private:
    std::string owner_;
    int balance_;
};

std::optional<Account> find_account(const std::vector<Account>& accounts, const std::string& owner)
{
    for (const auto& account : accounts) {
        if (account.owner() == owner) {
            return account;
        }
    }
    return std::nullopt;
}

int main()
{
    auto primary = std::make_unique<Account>("kim", 10000);
    primary->deposit(5000);
    primary->withdraw(3000);

    const std::vector<Account> accounts = {
        *primary,
        Account{"lee", 7000},
        Account{"park", 12000},
    };

    const auto found = find_account(accounts, "kim");
    if (found.has_value()) {
        std::cout << found->owner() << ": " << found->balance() << '\n';
    }

    auto shared = std::make_shared<Account>("shared-owner", 1);
    auto alias = shared;
    std::cout << "shared count: " << shared.use_count() << '\n';
    std::cout << alias->owner() << '\n';

    return 0;
}
