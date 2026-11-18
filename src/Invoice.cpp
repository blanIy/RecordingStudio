#include "Invoice.h"

#include <iostream>
#include <sstream>

#include "Exceptions.h"
#include "Utils.h"

Invoice::Invoice(int id, int sessionId, int clientId, double amount, bool paid, std::string paymentDate)
    : sessionId_(sessionId), clientId_(clientId), amount_(amount), paid_(paid), paymentDate_(std::move(paymentDate)) {
    setId(id);
    if (sessionId_ <= 0 || clientId_ <= 0) throw ValidationException("счёт должен ссылаться на сессию и клиента");
    if (amount_ < 0.0) throw ValidationException("сумма счёта не может быть отрицательной");
    if (paid_ != !paymentDate_.empty()) throw ValidationException("дата оплаты должна быть указана только для оплаченного счёта");
}

void Invoice::setId(int id) {
    if (id < 0) throw ValidationException("id не может быть отрицательным");
    id_ = id;
}

void Invoice::processPayment(const DateTime& when) {
    if (paid_) throw ConstraintViolationException("счёт #" + std::to_string(id_) + " уже оплачен");
    paid_ = true;
    paymentDate_ = when.toString();
}

std::string Invoice::getReceiptDetails() const {
    std::ostringstream o;
    o << "=== КВИТАНЦИЯ № " << id_ << " ===\n"
      << "Сессия: #" << sessionId_ << "\nКлиент: #" << clientId_ << "\nСумма: " << util::formatMoney(amount_)
      << "\nСтатус: " << (paid_ ? "оплачен " + paymentDate_ : "ожидает оплаты") << "\n";
    return o.str();
}

std::string Invoice::toRecord() const {
    return "INVOICE|" + std::to_string(id_) + "|" + std::to_string(sessionId_) + "|" + std::to_string(clientId_) +
           "|" + util::num(amount_) + "|" + (paid_ ? "1" : "0") + "|" + util::escape(paymentDate_);
}

Invoice Invoice::fromFields(const std::vector<std::string>& f) {
    if (f.size() != 6) throw ValidationException("запись «INVOICE» должна содержать 6 полей");
    return Invoice(util::parseInt(f[0], "id"), util::parseInt(f[1], "сессия"), util::parseInt(f[2], "клиент"),
                   util::parseDouble(f[3], "сумма"), util::parseBool(f[4]), f[5]);
}

std::ostream& operator<<(std::ostream& os, const Invoice& i) {
    return os << "[Счёт #" << i.getId() << "] сессия #" << i.getSessionId() << " | клиент #" << i.getClientId()
              << " | " << util::formatMoney(i.getAmount()) << " | "
              << (i.isPaid() ? "оплачен " + i.getPaymentDate() : "не оплачен");
}
