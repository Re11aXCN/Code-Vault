#pragma once

#include <queue>
#include <string>
#include <future>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <iostream>
#include <unordered_map>
#include <functional>
#include <random>
#include <atomic>

// 消息类型
enum class MessageType {
    InsertCard,     // 插卡
    EjectCard,      // 退卡
    EnterPin,       // 输入密码
    VerifyPin,      // 验证密码
    SelectOperation,// 选择操作
    WithdrawRequest,// 取款请求
    WithdrawResult, // 取款结果
    BalanceInquiry, // 查询余额
    DisplayMessage  // 显示消息
};

// 消息结构
struct Message {
    MessageType type;
    std::string cardNumber;
    int amount = 0;
    int pin = 0;
    bool success = false;
    std::string accountId;
    int balance = 0;
    std::string text; // 显示消息
};

// 通道类
class Channel {
public:
    void send(const Message& msg) {
        std::unique_lock<std::mutex> lock(mutex_);
        queue_.push(msg);
        cv_.notify_one();
    }

    Message receive() {
        std::unique_lock<std::mutex> lock(mutex_);
        cv_.wait(lock, [this] { return !queue_.empty(); });
        Message msg = queue_.front();
        queue_.pop();
        return msg;
    }

    bool try_receive(Message& msg) {
        std::unique_lock<std::mutex> lock(mutex_);
        if (queue_.empty()) return false;
        msg = queue_.front();
        queue_.pop();
        return true;
    }

private:
    std::queue<Message> queue_;
    std::mutex mutex_;
    std::condition_variable cv_;
};

// 银行账户
struct Account {
    std::string id;
    std::string cardNumber;
    int pin;
    int balance;
};

// 银行服务
class BankService {
public:
    BankService() {
        // 初始化测试账户
        accounts_["acc-001"] = { "acc-001", "1234-5678", 1234, 5000 };
        accounts_["acc-002"] = { "acc-002", "8765-4321", 4321, 10000 };
    }

    bool verifyCard(const std::string& cardNumber) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100)); // 模拟处理时间
        for (const auto& [id, acc] : accounts_) {
            if (acc.cardNumber == cardNumber) {
                return true;
            }
        }
        return false;
    }

    bool verifyPin(const std::string& cardNumber, int pin) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100)); // 模拟处理时间
        for (const auto& [id, acc] : accounts_) {
            if (acc.cardNumber == cardNumber && acc.pin == pin) {
                currentAccount_ = acc;
                return true;
            }
        }
        return false;
    }

    bool withdraw(int amount) {
        std::this_thread::sleep_for(std::chrono::milliseconds(200)); // 模拟处理时间
        if (currentAccount_.id.empty()) return false;
        if (amount <= 0 || amount > currentAccount_.balance) return false;

        accounts_[currentAccount_.id].balance -= amount;
        currentAccount_.balance -= amount;
        return true;
    }

    int getBalance() const {
        if (currentAccount_.id.empty()) return -1;
        return currentAccount_.balance;
    }

private:
    std::unordered_map<std::string, Account> accounts_;
    Account currentAccount_;
};

// ATM服务
class ATMService {
public:
    ATMService(Channel& uiChannel, Channel& bankChannel)
        : uiChannel_(uiChannel), bankChannel_(bankChannel) {
    }

    void run() {
        while (true) {
            auto msg = uiChannel_.receive();

            switch (msg.type) {
            case MessageType::InsertCard:
                handleInsertCard(msg);
                break;
            case MessageType::EnterPin:
                handleEnterPin(msg);
                break;
            case MessageType::SelectOperation:
                handleSelectOperation(msg);
                break;
            case MessageType::WithdrawRequest:
                handleWithdrawRequest(msg);
                break;
            case MessageType::BalanceInquiry:
                handleBalanceInquiry();
                break;
            default:
                break;
            }
        }
    }

private:
    void handleInsertCard(const Message& msg) {
        // 发送验证请求到银行
        Message verifyMsg{ MessageType::VerifyPin };
        verifyMsg.cardNumber = msg.cardNumber;
        bankChannel_.send(verifyMsg);

        // 请求用户输入PIN
        Message enterPinMsg{ MessageType::DisplayMessage };
        enterPinMsg.text = "Please enter your PIN:";
        uiChannel_.send(enterPinMsg);
    }

    void handleEnterPin(const Message& msg) {
        // 发送PIN验证到银行
        Message verifyPinMsg{ MessageType::VerifyPin };
        verifyPinMsg.cardNumber = currentCard_;
        verifyPinMsg.pin = msg.pin;
        bankChannel_.send(verifyPinMsg);

        // 等待银行响应
        auto response = bankChannel_.receive();

        if (response.success) {
            // PIN验证成功
            Message successMsg{ MessageType::DisplayMessage };
            successMsg.text = "PIN verified. Select operation:";
            successMsg.text += "\n1. Withdraw\n2. Balance Inquiry\n3. Exit";
            uiChannel_.send(successMsg);
        }
        else {
            // PIN验证失败
            pinAttempts_++;

            if (pinAttempts_ >= 3) {
                // 超过尝试次数，退卡
                Message ejectMsg{ MessageType::DisplayMessage };
                ejectMsg.text = "Too many failed attempts. Card ejected.";
                uiChannel_.send(ejectMsg);

                Message ejectCard{ MessageType::EjectCard };
                uiChannel_.send(ejectCard);

                resetSession();
            }
            else {
                // 请求重新输入PIN
                Message retryMsg{ MessageType::DisplayMessage };
                retryMsg.text = "Invalid PIN. Please try again:";
                uiChannel_.send(retryMsg);
            }
        }
    }

    void handleSelectOperation(const Message& msg) {
        if (msg.amount == 1) { // Withdraw
            Message operationMsg{ MessageType::DisplayMessage };
            operationMsg.text = "Enter amount to withdraw:";
            uiChannel_.send(operationMsg);
        }
        else if (msg.amount == 2) { // Balance Inquiry
            handleBalanceInquiry();
        }
        else { // Exit
            Message exitMsg{ MessageType::DisplayMessage };
            exitMsg.text = "Thank you. Card ejected.";
            uiChannel_.send(exitMsg);

            Message ejectCard{ MessageType::EjectCard };
            uiChannel_.send(ejectCard);

            resetSession();
        }
    }

    void handleWithdrawRequest(const Message& msg) {
        // 发送取款请求到银行
        Message withdrawMsg{ MessageType::WithdrawRequest };
        withdrawMsg.amount = msg.amount;
        bankChannel_.send(withdrawMsg);

        // 等待银行响应
        auto response = bankChannel_.receive();

        if (response.success) {
            // 取款成功
            Message successMsg{ MessageType::DisplayMessage };
            successMsg.text = "Withdrawal successful. Dispensing $" +
                std::to_string(msg.amount);
            uiChannel_.send(successMsg);

            // 显示余额
            handleBalanceInquiry();
        }
        else {
            // 取款失败
            Message failMsg{ MessageType::DisplayMessage };
            failMsg.text = "Withdrawal failed. Insufficient funds.";
            uiChannel_.send(failMsg);
        }
    }

    void handleBalanceInquiry() {
        // 发送余额查询到银行
        Message balanceMsg{ MessageType::BalanceInquiry };
        bankChannel_.send(balanceMsg);

        // 等待银行响应
        auto response = bankChannel_.receive();

        Message displayMsg{ MessageType::DisplayMessage };
        displayMsg.text = "Current balance: $" +
            std::to_string(response.balance);
        uiChannel_.send(displayMsg);
    }

    void resetSession() {
        currentCard_.clear();
        pinAttempts_ = 0;
    }

    Channel& uiChannel_;
    Channel& bankChannel_;
    std::string currentCard_;
    int pinAttempts_ = 0;
};

// 用户界面
class UserInterface {
public:
    UserInterface(Channel& atmChannel) : atmChannel_(atmChannel) {}

    void run() {
        std::cout << "=== ATM Simulator ===" << std::endl;

        while (true) {
            std::cout << "\nOptions:\n"
                << "1. Insert card\n"
                << "2. Exit\n"
                << "Select: ";

            int choice;
            std::cin >> choice;

            if (choice == 1) {
                startSession();
            }
            else if (choice == 2) {
                break;
            }
        }
    }

private:
    void startSession() {
        std::string cardNumber;
        std::cout << "Enter card number (e.g., 1234-5678): ";
        std::cin >> cardNumber;

        // 发送插卡消息
        Message insertMsg{ MessageType::InsertCard };
        insertMsg.cardNumber = cardNumber;
        atmChannel_.send(insertMsg);

        // 处理ATM消息
        while (true) {
            Message msg;
            if (displayChannel_.try_receive(msg)) {
                if (msg.type == MessageType::DisplayMessage) {
                    std::cout << "\nATM: " << msg.text << std::endl;
                }
                else if (msg.type == MessageType::EjectCard) {
                    std::cout << "ATM: Card ejected." << std::endl;
                    return;
                }
            }

            // 检查是否有用户输入
            if (msg.type == MessageType::EnterPin ||
                msg.type == MessageType::SelectOperation ||
                msg.type == MessageType::WithdrawRequest) {
                continue;
            }

            std::string input;
            std::cout << "> ";
            std::cin >> input;

            if (input == "exit") {
                Message exitMsg{ MessageType::EjectCard };
                atmChannel_.send(exitMsg);
                return;
            }

            try {
                if (input.find_first_not_of("0123456789") == std::string::npos) {
                    int value = std::stoi(input);

                    // 根据上下文发送不同类型消息
                    if (waitingForPin_) {
                        Message pinMsg{ MessageType::EnterPin };
                        pinMsg.pin = value;
                        atmChannel_.send(pinMsg);
                        waitingForPin_ = false;
                    }
                    else if (waitingForOperation_) {
                        Message opMsg{ MessageType::SelectOperation };
                        opMsg.amount = value;
                        atmChannel_.send(opMsg);
                        waitingForOperation_ = false;

                        if (value == 1) waitingForAmount_ = true;
                    }
                    else if (waitingForAmount_) {
                        Message amountMsg{ MessageType::WithdrawRequest };
                        amountMsg.amount = value;
                        atmChannel_.send(amountMsg);
                        waitingForAmount_ = false;
                    }
                }
            }
            catch (...) {
                std::cout << "Invalid input" << std::endl;
            }
        }
    }

    Channel& atmChannel_;
    Channel displayChannel_;
    bool waitingForPin_ = false;
    bool waitingForOperation_ = false;
    bool waitingForAmount_ = false;
};

int main() {
    // 创建通道
    Channel uiToAtm;
    Channel atmToBank;
    Channel bankToAtm;
    Channel atmToUi;

    // 创建银行服务
    BankService bankService;
    std::thread bankThread([&] {
        while (true) {
            auto msg = atmToBank.receive();

            switch (msg.type) {
            case MessageType::VerifyPin: {
                Message response{ MessageType::VerifyPin };
                response.success = bankService.verifyPin(msg.cardNumber, msg.pin);
                bankToAtm.send(response);
                break;
            }
            case MessageType::WithdrawRequest: {
                Message response{ MessageType::WithdrawResult };
                response.success = bankService.withdraw(msg.amount);
                bankToAtm.send(response);
                break;
            }
            case MessageType::BalanceInquiry: {
                Message response{ MessageType::BalanceInquiry };
                response.balance = bankService.getBalance();
                bankToAtm.send(response);
                break;
            }
            default:
                break;
            }
        }
        });

    // 创建ATM服务
    ATMService atmService(atmToUi, bankToAtm);
    std::thread atmThread([&] {
        atmService.run();
        });

    // 创建用户界面
    UserInterface ui(uiToAtm);
    ui.run();

    // 清理
    bankThread.detach();
    atmThread.detach();

    return 0;
}