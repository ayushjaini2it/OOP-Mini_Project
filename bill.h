#pragma once
#include "inventory.h"

class Bill
{
    string billID;
    string customerName;
    string customerPhoneNumber;
    vector<CartItem> cartList;
    double totalTax = 0;
    double totalDiscount = 0;
    double total = 0;
    double finalTotal = 0;

    bool ensureTransactionSchema(const string &fileName)
    {
        const string legacyHeader =
            "bill_id,customer_name,item_id,item_name,quantity,unit_price,"
            "tax_percent,discount_percent,subtotal,tax_amount,discount_amount,final_total";
        const string currentHeader = legacyHeader + ",customer_phone";

        ifstream existingFile(fileName.c_str());
        if (!existingFile)
            return true;

        string header;
        if (!getline(existingFile, header) || header == currentHeader)
            return true;
        if (header != legacyHeader)
        {
            error("Could not update " + fileName + ": unrecognized transaction header.");
            return false;
        }

        ofstream updatedFile("transactions.tmp");
        if (!updatedFile)
        {
            error("Could not create a temporary transaction file.");
            return false;
        }

        updatedFile << currentHeader << '\n';
        string line;
        while (getline(existingFile, line))
        {
            bool validRow;
            vector<string> fields = parseCsvRow(line, validRow);
            if (!validRow || fields.size() != 12)
            {
                updatedFile.close();
                std::remove("transactions.tmp");
                error("Could not update " + fileName + ": an existing transaction row is invalid.");
                return false;
            }

            for (size_t i = 0; i < fields.size(); ++i)
            {
                if (i != 0)
                    updatedFile << ',';
                updatedFile << csvField(fields[i]);
            }
            updatedFile << ",\n";
        }
        existingFile.close();
        updatedFile.flush();
        updatedFile.close();
        if (!updatedFile)
        {
            std::remove("transactions.tmp");
            error("Could not finish updating " + fileName + ".");
            return false;
        }

        if (std::remove(fileName.c_str()) != 0)
        {
            std::remove("transactions.tmp");
            error("Could not replace " + fileName + ".");
            return false;
        }
        if (rename("transactions.tmp", fileName.c_str()) != 0)
        {
            error("Could not finish updating " + fileName + ".");
            return false;
        }
        return true;
    }

public:
    Bill(string billID, string customerName, string customerPhoneNumber, vector<CartItem> cartList)
        : billID(billID), customerName(customerName),
          customerPhoneNumber(customerPhoneNumber), cartList(cartList) {}

    void calculateBill()
    {
        totalTax = 0;
        totalDiscount = 0;
        total = 0;

        for (auto &item : cartList)
        {
            double itemTotal = item.getPrice() * item.getQuantity();
            total += itemTotal;
            totalTax += itemTotal * (item.getTax() / 100);
            totalDiscount += itemTotal * (item.getDiscount() / 100);
        }

        finalTotal = total + totalTax - totalDiscount;
    }

    void displayBill()
    {
        title("CHECKOUT RECEIPT");
        cout << "Receipt: " << billID << '\n';
        cout << "Customer: " << customerName << '\n';
        cout << "Phone: " << customerPhoneNumber << '\n';
        cout << string(56, '-') << '\n';
        cout << left << setw(12) << "ITEM ID"
             << setw(22) << "ITEM NAME" << right << setw(10) << "PRICE"
             << setw(12) << "DISCOUNT" << setw(12) << "QUANTITY" << '\n'
             << string(68, '-') << '\n';
        for (auto &item : cartList)
        {
            item.displayCartItem();
        }
        cout << string(56, '-') << '\n'
             << fixed << setprecision(2)
             << "  Subtotal       Rs " << total << '\n'
             << "  Tax            Rs " << totalTax << '\n'
             << "  Discount      -Rs " << totalDiscount << '\n'
             << "AMOUNT DUE: Rs " << finalTotal << '\n';
    }

    bool saveToCsv(const string &fileName)
    {
        if (!ensureTransactionSchema(fileName))
            return false;

        ifstream existingFile(fileName.c_str());
        bool needsHeader = !existingFile || existingFile.peek() == ifstream::traits_type::eof();
        existingFile.close();

        ofstream file(fileName.c_str(), ios::app);
        if (!file)
        {
            error("Could not write " + fileName + ".");
            return false;
        }

        if (needsHeader)
            file << "bill_id,customer_name,item_id,item_name,quantity,unit_price,"
                    "tax_percent,discount_percent,subtotal,tax_amount,discount_amount,final_total,"
                    "customer_phone\n";

        file << fixed << setprecision(2);
        for (CartItem &item : cartList)
        {
            double subtotal = item.getPrice() * item.getQuantity();
            double tax = subtotal * item.getTax() / 100;
            double discount = subtotal * item.getDiscount() / 100;
            file << csvField(billID) << ','
                 << csvField(customerName) << ','
                 << csvField(item.getId()) << ','
                 << csvField(item.getName()) << ','
                 << item.getQuantity() << ','
                 << item.getPrice() << ','
                 << item.getTax() << ','
                 << item.getDiscount() << ','
                 << subtotal << ','
                 << tax << ','
                 << discount << ','
                 << finalTotal << ','
                 << csvField(customerPhoneNumber) << '\n';
        }
        file.flush();
        if (!file)
        {
            error("Could not finish writing " + fileName + ".");
            return false;
        }
        return true;
    }
};
