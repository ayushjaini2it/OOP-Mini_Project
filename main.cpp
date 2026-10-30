#include "bill.h"
#include "customer.h"

int runProgram()
{
    Inventory inventory("inventory.csv");
    if (!inventory.createFileIfNeeded())
        return 1;
    int nextBillNumber = getNextBillNumber("transactions.csv");

    showMenu("WELCOME TO THE STORE", {"Customer", "Admin", "Exit"});
    while (true)
    {
        int userType = readMenuChoice(6);
        if (userType == 3)
        {
            break;
        }

        if (userType == 1)
        {
            Customer customer;
            Cart cart;
            success("Welcome, " + customer.getName() + "!");

            showMenu("CUSTOMER MENU", {
                "View inventory",
                "Add item to cart",
                "Remove item from cart",
                "Modify item in cart",
                "View cart",
                "Checkout",
                "Return to user selection"
            });
            bool customerDone = false;
            while (!customerDone)
            {
                int customerChoice = readMenuChoice(10);
                switch (customerChoice)
                {
                case 1:
                    inventory.displayItems();
                    break;
                case 2:
                {
                    string itemId;
                    itemId = readValue<string>("  Enter item ID: ");
                    Item item;
                    if (!inventory.findItem(itemId, item))
                    {
                        error("Item not found.");
                    }
                    else if (cart.addItem(CartItem(item, 1)))
                    {
                        success("Item added to cart.");
                    }
                    else
                    {
                        error("No more stock available for this item.");
                    }
                    break;
                }
                case 3:
                {
                    string itemId;
                    itemId = readValue<string>("  Enter item ID to remove: ");
                    if (cart.removeItem(itemId))
                        success("Item removed from cart.");
                    else
                        error("Item not found in cart.");
                    break;
                }
                case 4:
                {
                    string itemId;
                    char field;
                    itemId = readValue<string>("  Enter item ID to modify: ");
                    field = readValue<char>("  Action (q: change quantity, r: remove): ");
                    cart.modifyCartItem(itemId, field);
                    break;
                }
                case 5:
                {
                    vector<CartItem> cartList = cart.getCartList();
                    title("YOUR CART");
                    if (cartList.empty())
                    {
                        info("Cart is empty.");
                    }
                    else
                    {
                        cout << left << setw(12) << "ITEM ID"
                             << setw(22) << "ITEM NAME" << right << setw(10) << "PRICE"
                             << setw(12) << "DISCOUNT" << setw(12) << "QUANTITY" << '\n'
                             << string(68, '-') << '\n';
                    }
                    for (auto &item : cartList)
                    {
                        item.displayCartItem();
                    }
                    break;
                }
                case 6:
                {
                    vector<CartItem> cartList = cart.getCartList();
                    if (cartList.empty())
                    {
                        info("Cart is empty.");
                        break;
                    }
                    bool inStock = true;
                    for (auto &cartItem : cartList)
                    {
                        Item item;
                        if (!inventory.findItem(cartItem.getId(), item) ||
                            item.getStock() < cartItem.getQuantity())
                        {
                            inStock = false;
                            break;
                        }
                    }
                    if (!inStock)
                    {
                        error("Checkout failed: an item is no longer available in the requested quantity.");
                        break;
                    }
                    Bill bill("BILL" + to_string(nextBillNumber), customer.getName(),
                              customer.getPhoneNumber(), cartList);
                    bill.calculateBill();
                    if (!bill.saveToCsv("transactions.csv"))
                        break;
                    bool stockUpdated = true;
                    for (auto &cartItem : cartList)
                    {
                        if (!inventory.reduceStock(cartItem.getId(), cartItem.getQuantity()))
                        {
                            stockUpdated = false;
                            break;
                        }
                    }
                    if (!stockUpdated)
                    {
                        error("Could not update inventory stock. Please check the inventory file.");
                        break;
                    }
                    ++nextBillNumber;
                    bill.displayBill();
                    cart.clear();
                    success("Payment complete. Your cart is now empty.");
                    break;
                }
                case 7:
                    customerDone = true;
                    break;
                default:
                    error("Invalid choice.");
                    break;
                }
            }
            showMenu("WELCOME TO THE STORE", {"Customer", "Admin", "Exit"});
        }
        else if (userType == 2)
        {
            Admin admin("storeAdmin", "admin123");
            string username, password;
            title("ADMIN SIGN IN");
            username = readValue<string>("  Username: ");
            password = readValue<string>("  Password: ");
            if (!admin.login(username, password))
            {
                error("Invalid username or password.");
                continue;
            }

            success("Signed in as administrator.");
            showMenu("ADMIN MENU", {
                "Add item to inventory",
                "Remove item from inventory",
                "View inventory",
                "Display item details",
                "Modify item",
                "Return to user selection"
            });
            bool adminDone = false;
            while (!adminDone)
            {
                int adminChoice = readMenuChoice(9);
                switch (adminChoice)
                {
                case 1:
                {
                    string id, name;
                    double price, tax, discount;
                    int stock;
                    id = readValue<string>("  Item ID: ");
                    name = readValue<string>("  Item name: ");
                    price = readValue<double>("  Price: ");
                    stock = readValue<int>("  Stock: ");
                    tax = readValue<double>("  Tax percentage: ");
                    discount = readValue<double>("  Discount percentage: ");
                    if (!isfinite(price) || !isfinite(tax) || !isfinite(discount) ||
                        price < 0 || stock < 0 || tax < 0 || discount < 0 || discount > 100)
                    {
                        error("Invalid item values.");
                        break;
                    }
                    Item newItem(id, name, price, stock, tax, discount);
                    if (inventory.addItem(newItem))
                    {
                        success("Item added to inventory.");
                    }
                    else
                        error("An item with that ID already exists.");
                    break;
                }
                case 2:
                {
                    string itemId;
                    itemId = readValue<string>("  Enter item ID to remove: ");
                    if (inventory.removeItem(itemId))
                    {
                        success("Item removed from inventory.");
                    }
                    else
                        error("Item not found.");
                    break;
                }
                case 3:
                    inventory.displayItems();
                    break;
                case 4:
                {
                    string itemId;
                    itemId = readValue<string>("  Enter item ID to view: ");
                    inventory.displayItemDetails(itemId);
                    break;
                }
                case 5:
                {
                    string itemId;
                    itemId = readValue<string>("  Enter item ID to modify: ");
                    inventory.modifyItem(itemId);
                    break;
                }
                case 6:
                    adminDone = true;
                    break;
                default:
                    error("Invalid option.");
                    break;
                }
            }
            showMenu("WELCOME TO THE STORE", {"Customer", "Admin", "Exit"});
        }
        else
        {
            error("Invalid user type.");
        }
    }
    info("Thanks for shopping with us. Goodbye!");
    return 0;
}

int main()
{
    try
    {
        return runProgram();
    }
    catch (const runtime_error &exception)
    {
        error(string("The program stopped: ") + exception.what());
        return 1;
    }
}