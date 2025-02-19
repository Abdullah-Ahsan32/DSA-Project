#include <iomanip>
#include <iostream>
#include <cstdio>
#include <limits>
#include <string>
using namespace std;

// Constants
const int MAX_DAYS = 30; // Maximum days for room availability

// Define Room structure
struct Room
{
    int id;
    enum RoomType
    {
        SINGLE,
        DOUBLE,
        SUITE
    } type;
    enum Status
    {
        OCCUPIED,
        BOOKED,
        READY,
        UNAVAILABLE
    } status;
    int floorNumber;
    bool availabilityCalendar[MAX_DAYS]; // Availability for next 30 days

    Room(int id, RoomType type, Status status, int floor)
        : id(id), type(type), status(status), floorNumber(floor)
    {
        // Initialize all days as available
        for (int i = 0; i < MAX_DAYS; i++)
        {
            availabilityCalendar[i] = true;
        }
    }
};

// Define BookingRequest structure
struct BookingRequest
{
    string customerName;
    Room::RoomType requestedType;
    int nights;
    int floornum;
    int checkInDay; // Day number (0-29) for check-in
    bool isHighPriority;
    BookingRequest *next;
};

// Define BookingHistory structure
struct BookingHistory
{
    string customerName;
    Room::RoomType roomType;
    int roomId;
    int nights;
    BookingHistory *next;
};

// Define TreeNode for representing hotel floors and rooms
struct TreeNode
{
    Room *room;
    TreeNode *left;
    TreeNode *right;
    TreeNode *parent;

    TreeNode(Room *r) : room(r), left(nullptr), right(nullptr), parent(nullptr) {}
};

// Custom Queue Implementation
struct BookingQueue
{
    BookingRequest *front;
    BookingRequest *rear;

    BookingQueue() : front(nullptr), rear(nullptr) {}

    void enqueue(BookingRequest *request)
    {
        if (!rear)
        {
            front = rear = request;
        }
        else
        {
            rear->next = request;
            rear = request;
        }
    }

    BookingRequest *dequeue()
    {
        if (!front)
            return nullptr;
        BookingRequest *request = front;
        front = front->next;
        if (!front)
            rear = nullptr;
        return request;
    }

    bool isEmpty()
    {
        return front == nullptr;
    }
};

// Custom Stack Implementation
struct BookingStack
{
    BookingHistory *top;

    BookingStack() : top(nullptr) {}

    void push(BookingHistory *history)
    {
        history->next = top;
        top = history;
    }

    BookingHistory *pop()
    {
        if (!top)
            return nullptr;
        BookingHistory *history = top;
        top = top->next;
        return history;
    }

    bool isEmpty()
    {
        return top == nullptr;
    }

    void display()
    {
        cout << "\n--- Booking History ---\n";
        cout << left << setw(20) << "Customer Name"
             << setw(10) << "Room ID"
             << setw(15) << "Room Type"
             << setw(10) << "Nights" << endl;

        cout << setfill('-') << setw(70) << "" << setfill(' ') << endl; // Separator line

        BookingHistory *current = top;
        if (!current)
        {
            cout << "No bookings in the history.\n";
            return;
        }
        while (current)
        {
            cout << left << setw(20) << current->customerName
                 << setw(10) << current->roomId
                 << setw(15) << (current->roomType == Room::SINGLE ? "Single" : current->roomType == Room::DOUBLE ? "Double"
                                                                                                                  : "Suite")
                 << setw(10) << current->nights << "\n";
            current = current->next;
        }
    }
};

// Hotel Management System Class
class HotelManagementSystem
{
private:
    TreeNode *root;             // Tree root for hotel floors and rooms
    BookingQueue regularQueue;  // Regular booking queue
    BookingQueue priorityQueue; // Priority booking queue
    BookingStack historyStack;  // Stack for booking history

    // Helper function to insert room into the tree
    TreeNode *insertRoom(TreeNode *node, TreeNode *newNode)
    {
        if (!node)
            return newNode;
        if (newNode->room->id < node->room->id)
        {
            node->left = insertRoom(node->left, newNode);
            node->left->parent = node;
        }
        else
        {
            node->right = insertRoom(node->right, newNode);
            node->right->parent = node;
        }
        return node;
    }

    // Cleanup function for tree
    void cleanupTree(TreeNode *node)
    {
        if (!node)
            return;
        cleanupTree(node->left);
        cleanupTree(node->right);
        delete node->room;
        delete node;
    }

    // Updated function to check room availability for specific dates
    bool isRoomAvailable(Room *room, int checkInDay, int nights)
    {
        if (checkInDay + nights > MAX_DAYS)
            return false;

        for (int i = checkInDay; i < checkInDay + nights; i++)
        {
            if (!room->availabilityCalendar[i])
                return false;
        }
        return true;
    }


    // Display room status (in-order traversal)

    void displayRoomStatusHelper(TreeNode *node)
    {
        if (!node)
            return;

        displayRoomStatusHelper(node->left);

        cout << left << setw(10) << node->room->id
             << setw(15) << (node->room->type == Room::SINGLE ? "Single" : node->room->type == Room::DOUBLE ? "Double"
                                                                                                            : "Suite")
             << setw(15) << node->room->floorNumber
             << setw(15) << (node->room->status == Room::OCCUPIED ? "Occupied" : node->room->status == Room::BOOKED ? "Booked"
                                                                             : node->room->status == Room::READY    ? "Ready"
                                                                                                                    : "Unavailable")
             << endl;

        displayRoomStatusHelper(node->right);
    }
    // Process a single booking request
    void processSingleRequest(BookingRequest *request)
    {
        TreeNode *availableRoom = findAvailableRoomOnFloor(root, request->requestedType,
                                                           request->floornum, request->checkInDay,
                                                           request->nights);

        if (availableRoom)
        {
            // Mark room as booked for the specified dates
            for (int i = request->checkInDay; i < request->checkInDay + request->nights; i++)
            {
                availableRoom->room->availabilityCalendar[i] = false;
            }
            availableRoom->room->status = Room::BOOKED;

            // Create booking history record
            BookingHistory *history = new BookingHistory{
                request->customerName,
                request->requestedType,
                availableRoom->room->id,
                request->nights,
                nullptr};
            historyStack.push(history);

            cout << "Booking confirmed for " << request->customerName
                 << "\nRoom ID: " << availableRoom->room->id
                 << "\nFloor: " << availableRoom->room->floorNumber
                 << "\nCheck-in Day: " << request->checkInDay
                 << "\nNights: " << request->nights << endl;
        }
        else
        {
            cout << "No available room found for " << request->customerName
                 << " on floor " << request->floornum << endl;
        }
        delete request;
    }
    // Cancel most recent booking
    void cancelMostRecentBooking()
    {
        if (historyStack.isEmpty())
        {
            cout << "No bookings to cancel.\n";
            return;
        }

        BookingHistory *history = historyStack.pop();
        TreeNode *roomNode = findRoomById(root, history->roomId);

        if (roomNode)
        {
            // Mark room as available for the specified dates
            for (int i = 0; i < history->nights; i++)
            {
                roomNode->room->availabilityCalendar[i] = true;
            }
            roomNode->room->status = Room::READY;

            cout << "Cancelled booking for customer: " << history->customerName
                 << ", Room ID: " << history->roomId << ", Nights: " << history->nights << "\n";
        }

        delete history;
    }

    // Helper function to find a room by ID
    TreeNode *findRoomById(TreeNode *node, int roomId)
    {
        if (!node)
            return nullptr;
        if (node->room->id == roomId)
            return node;
        TreeNode *leftSearch = findRoomById(node->left, roomId);
        if (leftSearch)
            return leftSearch;
        return findRoomById(node->right, roomId);
    }

    void displayRoomsByFloorHelper(TreeNode *node, int floorNumber)
    {
        if (!node)
            return;

        displayRoomsByFloorHelper(node->left, floorNumber);

        if (node->room->floorNumber == floorNumber)
        {
            cout << "Room ID: " << node->room->id
                 << ", Type: " << (node->room->type == Room::SINGLE ? "Single" : node->room->type == Room::DOUBLE ? "Double"
                                                                                                                  : "Suite")
                 << ", Status: ";
            switch (node->room->status)
            {
            case Room::OCCUPIED:
                cout << "Occupied";
                break;
            case Room::BOOKED:
                cout << "Booked";
                break;
            case Room::READY:
                cout << "Ready";
                break;
            default:
                cout << "Unavailable";
                break;
            }
            cout << "\n";
        }

        displayRoomsByFloorHelper(node->right, floorNumber);
    }
    TreeNode *findAvailableRoomOnFloor(TreeNode *node, Room::RoomType type, int floorNumber, int checkInDay, int nights)
    {
        if (!node)
            return nullptr;


        // Check if the room is on the specified floor and matches the requested type
        if (node->room->floorNumber == floorNumber && node->room->type == type &&
            node->room->status == Room::READY && isRoomAvailable(node->room, checkInDay, nights))
        {
            return node;
        }

        // Search left and right subtrees
        TreeNode *leftSearch = findAvailableRoomOnFloor(node->left, type, floorNumber, checkInDay, nights);
        if (leftSearch)
            return leftSearch;

        return findAvailableRoomOnFloor(node->right, type, floorNumber, checkInDay, nights);
    }

public:
    // Constructor
    HotelManagementSystem(int floors, int roomsPerFloor)
        : root(nullptr)
    {
        int idCounter = 1;

        for (int floor = 1; floor <= floors; ++floor)
        {
            // Ensure even distribution of room types per floor
            int singlesPerFloor = roomsPerFloor / 3;
            int doublesPerFloor = roomsPerFloor / 3;
            int suitesPerFloor = roomsPerFloor - singlesPerFloor - doublesPerFloor;

            // Create single rooms
            for (int i = 0; i < singlesPerFloor; ++i)
            {
                Room *newRoom = new Room(idCounter++, Room::SINGLE, Room::READY, floor);
                TreeNode *newNode = new TreeNode(newRoom);
                root = insertRoom(root, newNode);
            }

            // Create double rooms
            for (int i = 0; i < doublesPerFloor; ++i)
            {
                Room *newRoom = new Room(idCounter++, Room::DOUBLE, Room::READY, floor);
                TreeNode *newNode = new TreeNode(newRoom);
                root = insertRoom(root, newNode);
            }

            // Create suites
            for (int i = 0; i < suitesPerFloor; ++i)
            {
                Room *newRoom = new Room(idCounter++, Room::SUITE, Room::READY, floor);
                TreeNode *newNode = new TreeNode(newRoom);
                root = insertRoom(root, newNode);
            }
        }
    }
    void displayRoomsByFloor(int floorNumber)
    {
        cout << "\nRooms on Floor " << floorNumber << ":\n";
        displayRoomsByFloorHelper(root, floorNumber);
    }

    // Destructor
    ~HotelManagementSystem()
    {
        cleanupTree(root);
        while (historyStack.top)
        {
            delete historyStack.pop();
        }
    }

    // Make a booking request

    void makeBookingRequest(string name, Room::RoomType type, int nights,
                            int checkInDay, bool isPriority, int floorNumber)
    {
        // Input validation
        if (checkInDay < 0 || checkInDay >= MAX_DAYS || nights <= 0 ||
            checkInDay + nights > MAX_DAYS || floorNumber < 1 || floorNumber > 5)
        {
            cout << "Invalid booking parameters!\n";
            return;
        }

        cout << "\nChecking availability for:\n"
             << "Customer: " << name << "\n"
             << "Floor: " << floorNumber << "\n"
             << "Room Type: " << (type == Room::SINGLE ? "Single" : type == Room::DOUBLE ? "Double"
                                                                                         : "Suite")
             << "\n"
             << "Check-in Day: " << checkInDay << "\n"
             << "Nights: " << nights << "\n";

        // First check if there's an available room of the requested type on the specified floor
        TreeNode *availableRoom = findAvailableRoomOnFloor(root, type, floorNumber, checkInDay, nights);

        if (availableRoom)
        {
            // Room is available, create the booking request
            BookingRequest *request = new BookingRequest{
                name, type, nights, floorNumber, checkInDay, isPriority, nullptr};

            // Add to appropriate queue
            if (isPriority)
            {
                priorityQueue.enqueue(request);
                cout << "High-priority booking request added successfully.\n";
            }
            else
            {
                regularQueue.enqueue(request);
                cout << "Regular booking request added successfully.\n";
            }

            cout << "\nRoom details:"
                 << "\nRoom ID: " << availableRoom->room->id
                 << "\nFloor: " << availableRoom->room->floorNumber
                 << "\nType: " << (availableRoom->room->type == Room::SINGLE ? "Single" : availableRoom->room->type == Room::DOUBLE ? "Double"
                                                                                                                                    : "Suite")
                 << endl;
        }
        else
        {
            cout << "\nSorry, no available rooms of type "
                 << (type == Room::SINGLE ? "Single" : type == Room::DOUBLE ? "Double"
                                                                            : "Suite")
                 << " on floor " << floorNumber
                 << " for the specified dates.\n"
                 << "Please try different dates or a different floor.\n";
        }
    }
    // Process booking requests
    void processBookingRequests()
    {
        int requestsProcessed = 0;

        // Handle high-priority requests first
        while (!priorityQueue.isEmpty() && requestsProcessed < 10)
        {
            BookingRequest *request = priorityQueue.dequeue();
            processSingleRequest(request);
            ++requestsProcessed;
        }

        // Handle regular requests
        while (!regularQueue.isEmpty() && requestsProcessed < 10)
        {
            BookingRequest *request = regularQueue.dequeue();
            processSingleRequest(request);
            ++requestsProcessed;
        }

        if (requestsProcessed == 0)
        {
            cout << "No requests to process.\n";
        }
    }
    void checkInCustomer(const string &customerName)
    {
        // First search in booking history
        BookingHistory *current = historyStack.top;
        bool found = false;
        int roomId = -1;
        int nights = 0;

        // Search for the most recent booking for this customer
        while (current)
        {
            if (current->customerName == customerName)
            {
                roomId = current->roomId;
                nights = current->nights;
                found = true;
                break;
            }
            current = current->next;
        }

        if (!found)
        {
            cout << "No booking found for customer: " << customerName << endl;
            return;
        }

        // Find the room in the tree
        TreeNode *roomNode = findRoomById(root, roomId);
        if (!roomNode)
        {
            cout << "Error: Room not found in system." << endl;
            return;
        }

        // Check if room is already occupied
        if (roomNode->room->status == Room::OCCUPIED)
        {
            cout << "Error: Room is already occupied." << endl;
            return;
        }

        // Update room status to occupied
        roomNode->room->status = Room::OCCUPIED;

        cout << "\nCheck-in successful!" << endl;
        cout << "Customer: " << customerName << endl;
        cout << "Room ID: " << roomId << endl;
        cout << "Floor: " << roomNode->room->floorNumber << endl;
        cout << "Room Type: " << (roomNode->room->type == Room::SINGLE ? "Single" : roomNode->room->type == Room::DOUBLE ? "Double"
                                                                                                                         : "Suite")
             << endl;
        cout << "Number of nights: " << nights << endl;
    }

    // Displaying booking requests
    void displayBookingRequests()
    {
        cout << "\n--- Current Booking Requests ---\n";
        cout << left << setw(20) << "Customer Name"
             << setw(15) << "Room Type"
             << setw(10) << "Floor"
             << setw(10) << "Nights"
             << setw(15) << "Check-In" << endl;

        cout << setfill('-') << setw(70) << "" << setfill(' ') << endl;

        // Display priority requests
        cout << "\nHigh-Priority Requests:\n";
        BookingRequest *current = priorityQueue.front;
        if (!current)
            cout << "No high-priority requests.\n";
        while (current)
        {
            cout << left << setw(20) << current->customerName
                 << setw(15) << (current->requestedType == Room::SINGLE ? "Single" : current->requestedType == Room::DOUBLE ? "Double"
                                                                                                                            : "Suite")
                 << setw(10) << current->floornum
                 << setw(10) << current->nights
                 << setw(15) << "Day " << current->checkInDay << endl;
            current = current->next;
        }

        // Display regular requests
        cout << "\nRegular Requests:\n";
        current = regularQueue.front;
        if (!current)
            cout << "No regular requests.\n";
        while (current)
        {
            cout << left << setw(20) << current->customerName
                 << setw(15) << (current->requestedType == Room::SINGLE ? "Single" : current->requestedType == Room::DOUBLE ? "Double"
                                                                                                                            : "Suite")
                 << setw(10) << current->floornum
                 << setw(10) << current->nights
                 << setw(15) << "Day " << current->checkInDay << endl;
            current = current->next;
        }
    }
    // Display booking history
    void displayBookingHistory()
    {
        historyStack.display();
    }

    // Display room status
    void displayRoomStatus()
    {
        cout << "\nRoom Status:\n";
        cout << left << setw(10) << "Room ID"
             << setw(15) << "Type"
             << setw(15) << "Floor"
             << setw(15) << "Status" << endl;

        cout << setfill('-') << setw(55) << "" << setfill(' ') << endl; // Separator line

        displayRoomStatusHelper(root);
    }
    // Cancel most recent booking
    void cancelMostRecentBookingRequest()
    {
        cancelMostRecentBooking();
    }
};
// Utility Class
class InputHandler
{
public:
    // Function to read an integer with validation
    static int readInt(const string &prompt)
    {
        int value;
        while (true)
        {
            cout << prompt;
            cin >> value;

            if (cin.fail())
            {
                cin.clear();                                         // Clear the error flag
                cin.ignore(numeric_limits<streamsize>::max(), '\n'); // Discard invalid input
                cout << "Invalid input! Please enter a valid integer.\n";
            }
            else
            {
                cin.ignore(numeric_limits<streamsize>::max(), '\n'); // Discard the rest of the line
                return value;
            }
        }
    }

    // Function to read a string
    static string readString(const string &prompt)
    {
        string value;
        cout << prompt;
        cin.ignore();
        getline(cin, value);
        return value;
    }

    // Function to read a boolean (1 for true, 0 for false)
    static bool readBool(const string &prompt)
    {
        int value;
        while (true)
        {
            value = readInt(prompt);
            if (value == 0 || value == 1)
            {
                return value == 1;
            }
            else
            {
                cout << "Invalid input! Please enter 1 (Yes) or 0 (No).\n";
            }
        }
    }
};
// Utility functions
void clearScreen()
{
    system("cls");
}

void waitForEnter()
{

    cin.clear();
    cin.ignore(numeric_limits<streamsize>::max(), '\n');

    cout << "Press Enter to continue...";
    cin.get();
}

int main()
{
    cout << endl<<endl;
    cout << "                                                                                ------------------------------------------------------------" << endl;
    cout << "                                                                                |                  <Welcome to Galaxy Hotel>               |" << endl;
    cout << "                                                                                ------------------------------------------------------------" << endl;
    cout << endl;
    cout << endl;

    cout << "      We are thrilled to welcome you to Galaxy Hotel, where elegance " << endl;
    cout << "      meets comfort." << endl;

    cout << endl;
    cout << "      At Galaxy Hotel, we offer:                                   " << endl;
    cout << "        1. Efficient room management across multiple levels        " << endl;
    cout << "        2. Smooth booking processes with priority options          " << endl;
    cout << "        3. Comprehensive booking history for tracking & auditing   " << endl;
    cout << "        4. A variety of room types to suit your preferences: Single, Double, " << endl;
    cout << "         and Suite accommodations for your relaxation.            " << endl;
    cout << endl;
    cout << "      Our aim is to ensure you have an unforgettable experience." << endl;
    cout << "      We hope you enjoy your stay. " << endl;
    cout << "     =========================================================================\n";

    HotelManagementSystem hotel(5, 10); // 5 floors, 10 rooms per floor
    int choice;

    while (true)
    {
        cout << endl;


   cout << endl;
cout << "=============================\n";
cout << "      Galaxy Hotel Menu       \n";
cout << "==============================\n";
cout << "   1. Make a booking request  \n";
cout << "   2. Display room status     \n";
cout << "   3. Display booking history  \n";
cout << "   4. Process booking requests  \n";
cout << "   5. Cancel most recent booking\n";
cout << "   6. Display rooms by floor   \n";
cout << "   7. Display booking requests   \n";
cout << "   8. Customer Check-in         \n";
cout << "   9. Exit                     \n";
cout << " ==============================\n";
cout << "   Enter your choice: ";

        // Input validation for menu choice
        if (!(cin >> choice))
        {
            cout << "Invalid input! Please enter a number between 1 and 6.\n";
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            continue;
        }

        if (choice < 1 || choice > 9)
        {
            cout << "Invalid choice! Please enter a number between 1 and 8.\n";
            continue;
        }

        switch (choice)
        {
        case 1:
        {
            clearScreen();
            cout << "\t\t\t\t-------------------------\n";
            cout << "\t\t\t\t REGISTER BOOKING REQUEST\n";
            cout << "\t\t\t\t-------------------------\n\n";

            string name = InputHandler::readString("Enter Customer Name: ");
            int floorNumber = InputHandler::readInt("Enter the floor number (1 to 5): ");
            if (floorNumber < 1 || floorNumber > 5) // Assuming 5 floors
            {
                cout << "Invalid floor number! Please enter a number between 1 and 5.\n";
                break;
            }

            int type;
            while (true)
            {
                type = InputHandler::readInt("Enter Room Type (0:Single, 1:Double, 2:Suite): ");
                if (type >= 0 && type <= 2)
                {
                    break;
                }
                else
                {
                    cout << "Invalid room type! Please enter 0 (Single), 1 (Double), or 2 (Suite).\n";
                }
            }

            int nights;
            while (true)
            {
                nights = InputHandler::readInt("Enter number of nights: ");
                if (nights > 0)
                {
                    break;
                }
                else
                {
                    cout << "Invalid number of nights! Please enter a positive value.\n";
                }
            }
            int checkInDay = InputHandler::readInt("Enter check-in day (0-29): ");

            // Validate check-in day
            if (checkInDay < 0 || checkInDay >= MAX_DAYS)
            {
                cout << "Invalid check-in day! Please enter a day between 0 and 29.\n";
                break;
            }
            int priority = InputHandler::readInt("Is this a high-priority request? (1: Yes, 0: No): ");

            hotel.makeBookingRequest(name, (Room::RoomType)type, nights, checkInDay, priority, floorNumber);
            waitForEnter();
            break;
        }

        case 2:
            clearScreen();
            cout << "\t\t\t\t------------------------\n";
            cout << "\t\t\t\tDISPLAYING ROOM STATUS\n";
            cout << "\t\t\t\t-------------------------\n\n";
            hotel.displayRoomStatus();
            waitForEnter();
            break;

        case 3:
            clearScreen();
            cout << "\t\t\t\t--------------------------\n";
            cout << "\t\t\t\tDISPLAYING BOOKING HISTORY\n";
            cout << "\t\t\t\t--------------------------\n\n";
            hotel.displayBookingHistory();
            waitForEnter();
            break;

        case 4:
            clearScreen();
            cout << "\t\t\t\t-------------------\n";
            cout << "\t\t\t\tPROCESSING REQUESTS\n";
            cout << "\t\t\t\t-------------------\n\n";
            hotel.processBookingRequests();
            waitForEnter();
            break;

        case 5:
            clearScreen();
            cout << "\t\t\t\t--------------------------\n";
            cout << "\t\t\t\tCANCEL MOST RECENT BOOKING\n";
            cout << "\t\t\t\t--------------------------\n\n";
            hotel.cancelMostRecentBookingRequest();
            waitForEnter();
            break;

        case 6:
        {
            clearScreen();
            cout << "\t\t\t\t------------------------\n";
            cout << "\t\t\t\tDISPLAYING ROOM ON FLOOR \n";
            cout << "\t\t\t\t------------------------\n\n";
            int floor;
            cout << "Enter floor number: ";
            cin >> floor;
            hotel.displayRoomsByFloor(floor);
            waitForEnter();
            break;
        }
        case 7:
        {
            clearScreen();
            cout << "\t\t\t\t---------------------------\n";
            cout << "\t\t\t\tDISPLAYING BOOKING REQUESTS\n";
            cout << "\t\t\t\t---------------------------\n\n";
            hotel.displayBookingRequests();
            waitForEnter();
            break;
        }
        case 8:
        {
            clearScreen();
            cout << "\t\t\t\t------------------\n";
            cout << "\t\t\t\tCUSTOMER CHECK-IN\n";
            cout << "\t\t\t\t------------------\n\n";
            string customerName = InputHandler::readString("Enter customer name for check-in: ");
            hotel.checkInCustomer(customerName);
            waitForEnter();
            break;
        }
        case 9:
        {
            cout << "Thank you for using Galaxy Hotel Management System. Exiting...\n";
            return 0;
        }
        }
    }
}
