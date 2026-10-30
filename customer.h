#pragma once
#include "inventory.h"

class Customer
{
private:
    string name;
    string phoneNumber;

public:
    Customer()
    {
        title("CUSTOMER DETAILS");
        cin.ignore();
        name = readLine("  Name: ");
        phoneNumber = readLine("  Phone number: ");
    }

    string getName()
    {
        return name;
    }

    string getPhoneNumber()
    {
        return phoneNumber;
    }
};

class Admin
{
private:
    string username;
    string password;

public:
    Admin(string username, string password)
        : username(username), password(password) {}

    bool login(string inputUsername, string inputPassword)
    {
        return inputUsername == username && inputPassword == password;
    }
};