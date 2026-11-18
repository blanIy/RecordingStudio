#pragma once
#include <iosfwd>
#include <string>
#include <vector>

#include "DateTime.h"

class Invoice {
public:
    Invoice() = default;
    Invoice(int id, int sessionId, int clientId, double amount, bool paid = false, std::string paymentDate = "");

    int getId() const { return id_; }
    int getSessionId() const { return sessionId_; }
    int getClientId() const { return clientId_; }
    double getAmount() const { return amount_; }
    bool isPaid() const { return paid_; }
    const std::string& getPaymentDate() const { return paymentDate_; }

    void setId(int id);
    void processPayment(const DateTime& when);
    std::string getReceiptDetails() const;

    std::string toRecord() const;
    static Invoice fromFields(const std::vector<std::string>& f);

    bool operator==(const Invoice& o) const { return id_ == o.id_; }

private:
    int id_ = 0, sessionId_ = 0, clientId_ = 0;
    double amount_ = 0.0;
    bool paid_ = false;
    std::string paymentDate_;
};

std::ostream& operator<<(std::ostream& os, const Invoice& i);
