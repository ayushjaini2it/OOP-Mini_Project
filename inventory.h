#pragma once
#include <iostream>
#include <fstream>
#include <iomanip>
#include <cmath>
#include <cstdlib>
#include <climits>
#include <string>
#include <vector>
#include <cctype>
#include <cstdio>
using namespace std;

void title(const string &text)
{
    cout << "\033[1;36m\n=== " << text << " ===\033[0m\n";
}

void option(const string &number, const string &text)
{
    cout << "\033[32m[" << number << "] " << text << "\033[0m\n";
}

void prompt(const string &text)
{
    cout << text;
}

void success(const string &text)
{
    cout << "\033[32mSuccess: " << text << "\033[0m\n";
}

void error(const string &text)
{
    cout << "\033[31mError: " << text << "\033[0m\n";
}

void info(const string &text)
{
    cout << text << '\n';
}

void showMenu(const string &heading, const vector<string> &options)
{
    cout << "\033[r\033[2J\033[H";

    title(heading);
    for (size_t i = 0; i < options.size(); ++i)
        option(to_string(i + 1), options[i]);

    int promptLine = static_cast<int>(options.size()) + 3;
    int firstResultLine = promptLine + 1;
    cout << "\033[" << firstResultLine << ";r";
    cout << "\033[" << promptLine << ";1H";
    cout << "Select an option: \033[0K";
    cout.flush();
}

template <typename T>
T readValue(const string &text)
{
    while (true)
    {
        prompt(text);
        T value;
        if (cin >> value)
            return value;

        if (cin.eof())
        {
            cout << "\nNo more input. Exiting program.\n";
            exit(0);
        }

        cin.clear();
        cin.ignore(1000, '\n');
        error("Invalid input. Please try again.");
    }
}

bool parseNumber(const string &text, int &value)
{
    char *end;
    long number = strtol(text.c_str(), &end, 10);
    if (end == text.c_str() || *end != '\0' || number < INT_MIN || number > INT_MAX)
        return false;

    value = static_cast<int>(number);
    return true;
}

bool parseNumber(const string &text, double &value)
{
    char *end;
    double number = strtod(text.c_str(), &end);
    if (end == text.c_str() || *end != '\0')
        return false;

    value = number;
    return true;
}

int readMenuChoice(int promptLine)
{
    while (true)
    {
        cout << "\033[" << promptLine << ";1H\033[2K";
        cout << "Select an option: \033[0K";
        int choice;
        if (cin >> choice)
        {
            cout << "\033[" << promptLine + 1 << ";1H\033[J";
            return choice;
        }

        if (cin.eof())
        {
            cout << "\nNo more input. Exiting program.\n";
            exit(0);
        }

        cin.clear();
        cin.ignore(1000, '\n');
        error("Invalid input. Please try again.");
    }
}

string readLine(const string &text)
{
    string value;
    while (true)
    {
        prompt(text);
        if (!getline(cin, value))
        {
            if (cin.eof())
            {
                cout << "\nNo more input. Exiting program.\n";
                exit(0);
            }

            error("Invalid input. Please try again.");
            continue;
        }
        if (value.find_first_not_of(" \t\r") != string::npos)
            return value;
        error("This value cannot be empty.");
    }
}

string csvField(const string &value)
{
    if (value.find_first_of(",\"\r\n") == string::npos)
        return value;

    string escaped = "\"";
    for (char character : value)
    {
        if (character == '"')
            escaped += '"';
        escaped += character;
    }
    return escaped + '"';
}

vector<string> parseCsvRow(const string &line, bool &valid)
{
    vector<string> fields;
    string field;
    bool quoted = false;
    valid = true;

    for (size_t i = 0; i < line.size(); ++i)
    {
        if (line[i] == '"')
        {
            if (quoted && i + 1 < line.size() && line[i + 1] == '"')
            {
                field += '"';
                ++i;
            }
            else
            {
                quoted = !quoted;
            }
        }
        else if (line[i] == ',' && !quoted)
        {
            fields.push_back(field);
            field.clear();
        }
        else
        {
            field += line[i];
        }
    }

    valid = !quoted;
    fields.push_back(field);
    return fields;
}

int getNextBillNumber(const string &fileName)
{
    ifstream file(fileName.c_str());
    if (!file)
        return 1;

    string line;
    getline(file, line);
    int highestNumber = 0;
    while (getline(file, line))
    {
        bool validRow;
        vector<string> fields = parseCsvRow(line, validRow);
        if (!validRow)
        {
            error("Skipping a transaction row with an invalid CSV format.");
            continue;
        }
        if (fields.empty() || fields[0].compare(0, 4, "BILL") != 0)
            continue;

        int number;
        if (!parseNumber(fields[0].substr(4), number))
        {
            error("Skipping a transaction row with an invalid bill ID.");
            continue;
        }
        if (number > highestNumber)
            highestNumber = number;
    }
    return highestNumber + 1;
}

class Item
{
private:
    string id;
    string name;
    double price;
    int stock;
    double tax;
    double discount;

public:
    Item() : price(0), stock(0), tax(0), discount(0) {}

    virtual ~Item() {}

    Item(string id, string name, double price, int stock, double tax, double discount)
    {
        this->id = id;
        this->name = name;
        this->price = price;
        this->stock = stock;
        this->tax = tax;
        this->discount = discount;
    }
    void itemDetails()
    {
        title("ITEM DETAILS");
        cout << "  ID       " << id << '\n'
             << "  Name     " << name << '\n'
             << "  Price    Rs" << fixed << setprecision(2) << price << '\n'
             << "  Stock    " << stock << '\n'
             << "  Tax      " << tax << "%\n"
             << "  Discount " << discount << "%\n";
    }
    virtual void displayItem()
    {
        cout << left << setw(12) << id
             << setw(22) << name << right << "Rs" << setw(9) << fixed << setprecision(2) << price
             << setw(12) << fixed << setprecision(2) << discount << "%" << '\n';
    }

    double getPrice()
    {
        return price;
    }
    double getTax()
    {
        return tax;
    }
    double getDiscount()
    {
        return discount;
    }
    int getStock()
    {
        return stock;
    }
    bool reduceStock(int quantity)
    {
        if (quantity <= 0 || quantity > stock)
        {
            return false;
        }
        stock -= quantity;
        return true;
    }
    string getId()
    {
        return id;
    }
    string getName()
    {
        return name;
    }

    bool modifyItem(char field, string newValue)
    {
        double doubleValue;
        int intValue;
        switch (tolower(field))
        {
        case 'n':
            name = newValue;
            return true;
        case 'p':
            if (parseNumber(newValue, doubleValue) && isfinite(doubleValue) && doubleValue >= 0)
            {
                price = doubleValue;
                return true;
            }
            break;
        case 's':
            if (parseNumber(newValue, intValue) && intValue >= 0)
            {
                stock = intValue;
                return true;
            }
            break;
        case 't':
            if (parseNumber(newValue, doubleValue) && isfinite(doubleValue) && doubleValue >= 0)
            {
                tax = doubleValue;
                return true;
            }
            break;
        case 'd':
            if (parseNumber(newValue, doubleValue) && isfinite(doubleValue) &&
                doubleValue >= 0 && doubleValue <= 100)
            {
                discount = doubleValue;
                return true;
            }
            break;
        default:
            error("Invalid field.");
            return false;
        }
        error("Enter a valid value for the selected field.");
        return false;
    }
};

bool readItem(const string &line, Item &item)
{
    bool validRow;
    vector<string> fields = parseCsvRow(line, validRow);
    double price, tax, discount;
    int stock;

    if (!validRow || fields.size() != 6 ||
        !parseNumber(fields[2], price) || !isfinite(price) || price < 0 ||
        !parseNumber(fields[3], stock) || stock < 0 ||
        !parseNumber(fields[4], tax) || !isfinite(tax) || tax < 0 ||
        !parseNumber(fields[5], discount) || !isfinite(discount) ||
        discount < 0 || discount > 100)
        return false;

    item = Item(fields[0], fields[1], price, stock, tax, discount);
    return true;
}

void writeItem(ofstream &file, Item &item)
{
    file << csvField(item.getId()) << ','
         << csvField(item.getName()) << ','
         << item.getPrice() << ','
         << item.getStock() << ','
         << item.getTax() << ','
         << item.getDiscount() << '\n';
}

class CartItem : public Item
{
private:
    int quantity;

public:
    CartItem(Item item, int quantity) : Item(item), quantity(quantity) {}

    void displayCartItem()
    {
        cout << left << setw(12) << getId()
             << setw(22) << getName() << right << "Rs" << setw(9) << fixed << setprecision(2) << getPrice()
             << setw(12) << fixed << setprecision(2) << getDiscount() << "%"
             << setw(12) << getQuantity() << '\n';
    }

    void displayItem() override
    {
        displayCartItem();
    }

    void setQuantity(int newQuantity)
    {
        quantity = newQuantity;
    }
    int getQuantity()
    {
        return quantity;
    }
};

class Cart
{
    vector<CartItem> cartList;

public:
    bool addItem(CartItem item)
    {
        if (item.getQuantity() <= 0 || item.getQuantity() > item.getStock())
        {
            return false;
        }
        for (auto &cartItem : cartList)
        {
            if (cartItem.getId() == item.getId())
            {
                int newQuantity = cartItem.getQuantity() + item.getQuantity();
                if (newQuantity > cartItem.getStock())
                {
                    return false;
                }
                cartItem.setQuantity(newQuantity);
                return true;
            }
        }
        cartList.push_back(item);
        return true;
    }
    bool removeItem(string itemId)
    {
        for (auto item = cartList.begin(); item != cartList.end(); ++item)
        {
            if (item->getId() == itemId)
            {
                cartList.erase(item);
                return true;
            }
        }
        return false;
    }
    
    bool modifyCartItem(string itemId, char field)
    {
        for (auto &item : cartList)
        {
            if (item.getId() == itemId)
            {
                if (field == 'q')
                {
                    int newQuantity;
                    newQuantity = readValue<int>("  Enter new quantity: ");
                    if (newQuantity <= 0 || newQuantity > item.getStock())
                    {
                        error("Invalid quantity or insufficient stock.");
                        return false;
                    }
                    item.setQuantity(newQuantity);
                    success("Cart quantity updated.");
                    return true;
                }
                else if (field == 'r')
                {
                    removeItem(itemId);
                    success("Item removed from cart.");
                    return true;
                }
                error("Invalid cart action.");
                return false;
            }
        }
        error("Item not found in cart.");
        return false;
    }
    vector<CartItem> getCartList()
    {
        return cartList;
    }
    void clear()
    {
        cartList.clear();
    }
};

class Inventory
{
private:
    string fileName;

public:
    Inventory(string fileName) : fileName(fileName) {}

    bool createFileIfNeeded()
    {
        ifstream file(fileName.c_str());
        if (!file)
        {
            ofstream newFile(fileName.c_str());
            if (!newFile)
            {
                error("Could not create " + fileName + ".");
                return false;
            }
            newFile << "id,name,price,stock,tax,discount\n";
            newFile.flush();
            if (!newFile)
            {
                error("Could not finish creating " + fileName + ".");
                return false;
            }
            return true;
        }
        return true;
    }

    bool findItem(string itemId, Item &result)
    {
        ifstream file(fileName.c_str());
        if (!file)
        {
            error("Could not open " + fileName + ".");
            return false;
        }

        string line;
        getline(file, line);
        while (getline(file, line))
        {
            Item item;
            if (readItem(line, item) && item.getId() == itemId)
            {
                result = item;
                return true;
            }
        }
        return false;
    }

    void displayItems()
    {
        ifstream file(fileName.c_str());
        if (!file)
        {
            error("Could not open " + fileName + ".");
            return;
        }

        title("STORE INVENTORY");
        cout << left << setw(12) << "ITEM ID"
             << setw(22) << "ITEM NAME" << right << setw(10) << "PRICE"
             << setw(12) << "DISCOUNT" << '\n'
             << string(56, '-') << '\n';

        string line;
        getline(file, line);
        bool found = false;
        while (getline(file, line))
        {
            Item item;
            if (readItem(line, item))
            {
                item.displayItem();
                found = true;
            }
        }
        if (!found)
        {
            info("Inventory is empty.");
        }
    }

    bool addItem(Item newItem)
    {
        Item existingItem;
        if (findItem(newItem.getId(), existingItem))
            return false;

        ofstream file(fileName.c_str(), ios::app);
        if (!file)
        {
            error("Could not write " + fileName + ".");
            return false;
        }
        writeItem(file, newItem);
        return file.good();
    }

    bool reduceStock(string itemId, int quantity)
    {
        Item item;
        if (!findItem(itemId, item) || !item.reduceStock(quantity))
            return false;
        return replaceItem(itemId, item, false);
    }

    bool removeItem(string itemId)
    {
        Item item;
        return findItem(itemId, item) && replaceItem(itemId, item, true);
    }

    void displayItemDetails(string itemId)
    {
        Item item;
        if (findItem(itemId, item))
        {
            item.itemDetails();
            return;
        }
        error("Item not found.");
    }

    bool modifyItem(string itemId)
    {
        Item item;
        if (!findItem(itemId, item))
        {
            error("Item not found.");
            return false;
        }

        item.itemDetails();
        info("Fields: Name[n], Price[p], Stock[s], Tax[t], Discount[d]");
        char field = readValue<char>("  Field to modify: ");
        string newValue = readValue<string>("  New value: ");
        if (!item.modifyItem(field, newValue))
            return false;

        if (replaceItem(itemId, item, false))
        {
            success("Item modified successfully.");
            return true;
        }
        return false;
    }

    bool replaceItem(string itemId, Item replacement, bool removeRow)
    {
        ifstream input(fileName.c_str());
        ofstream output("inventory.tmp");
        if (!input || !output)
        {
            error("Could not update the inventory file.");
            return false;
        }

        string line;
        getline(input, line);
        output << "id,name,price,stock,tax,discount\n";
        bool changed = false;

        while (getline(input, line))
        {
            Item currentItem;
            if (readItem(line, currentItem) && currentItem.getId() == itemId)
            {
                changed = true;
                if (!removeRow)
                    writeItem(output, replacement);
            }
            else
            {
                output << line << '\n';
            }
        }

        input.close();
        output.close();
        if (!changed || !output)
        {
            std::remove("inventory.tmp");
            error("Could not update the inventory file.");
            return false;
        }

        if (std::remove(fileName.c_str()) != 0)
        {
            std::remove("inventory.tmp");
            error("Could not replace the inventory file.");
            return false;
        }
        if (rename("inventory.tmp", fileName.c_str()) != 0)
        {
            error("Could not finish updating the inventory file.");
            return false;
        }
        return true;
    }
};
