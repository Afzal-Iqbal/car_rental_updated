#include "database.h"
#include <QSqlQuery>
#include <QDebug>

bool Database::initialize()
{
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName("car_rental.db");

    if (!db.open())
    {
        qDebug() << "Error: Could not open database.";
        return false;
    }

    QSqlQuery query;

    // Create Owners Table - Allow NULL for optional fields
    query.exec("CREATE TABLE IF NOT EXISTS owners ("
               "id INTEGER PRIMARY KEY AUTOINCREMENT, "
               "name TEXT NOT NULL, "
               "phone_number TEXT, "
               "address TEXT)");

    // // Insert Dummy Data into Owners Table
    // query.exec("INSERT INTO owners (name, phone_number, address) VALUES "
    //            "('John Doe', '123-456-7890', '123 Elm St, Springfield'), "
    //            "('Jane Smith', '987-654-3210', '456 Oak St, Rivertown'), "
    //            "('Alice Johnson', '555-123-4567', '789 Pine St, Hill Valley')");

    // Create Vehicles Table
    query.exec("CREATE TABLE IF NOT EXISTS vehicles ("
               "id INTEGER PRIMARY KEY AUTOINCREMENT, "
               "owner_id INTEGER NOT NULL, "
               "vehicle_name TEXT NOT NULL, "
               "make TEXT NOT NULL, "
               "model TEXT NOT NULL, "
               "year INTEGER NOT NULL, "
               "FOREIGN KEY (owner_id) REFERENCES owners(id) ON DELETE CASCADE)");

    // // Insert Dummy Data into Vehicles Table
    // query.exec("INSERT INTO vehicles (owner_id, vehicle_name, make, model, year) VALUES "
    //            "(1, 'Toyota Corolla', 'Toyota', 'Corolla', 2018), "
    //            "(2, 'Honda Civic', 'Honda', 'Civic', 2020), "
    //            "(3, 'Ford Mustang', 'Ford', 'Mustang', 2022)");

    // Create Customers Table
    query.exec("CREATE TABLE IF NOT EXISTS customers ("
               "id INTEGER PRIMARY KEY AUTOINCREMENT, "
               "name TEXT NOT NULL, "
               "phone_number TEXT NOT NULL, "
               "address TEXT NOT NULL)");

    // // Insert Dummy Data into Customers Table
    // query.exec("INSERT INTO customers (name, phone_number, address) VALUES "
    //            "('Mike Taylor', '333-555-1234', '12 Maple St, Smalltown'), "
    //            "('Susan Green', '444-666-7890', '34 Birch St, Lakeside'), "
    //            "('David Williams', '555-777-4321', '56 Cedar St, Oakfield')");

    // Create Bookings Table
    query.exec("CREATE TABLE IF NOT EXISTS bookings ("
               "id INTEGER PRIMARY KEY AUTOINCREMENT, "
               "owner_id INTEGER NOT NULL, "
               "vehicle_id INTEGER NOT NULL, "
               "customer_id INTEGER NOT NULL, "
               "booking_start TEXT NOT NULL, "
               "booking_end TEXT NOT NULL, "
               "amount_per_hour REAL NOT NULL, "
               "total_hours REAL NOT NULL, "
               "total_amount REAL NOT NULL, "
               "advance_amount REAL NOT NULL, "
               "pending_amount REAL NOT NULL, "
               "FOREIGN KEY (owner_id) REFERENCES owners(id) ON DELETE CASCADE, "
               "FOREIGN KEY (vehicle_id) REFERENCES vehicles(id) ON DELETE CASCADE, "
               "FOREIGN KEY (customer_id) REFERENCES customers(id) ON DELETE CASCADE)");

    // // Insert Dummy Data into Bookings Table
    // query.exec("INSERT INTO bookings (owner_id, vehicle_id, customer_id, booking_start, booking_end, amount_per_hour, total_hours, total_amount, advance_amount, pending_amount) VALUES "
    //            "(1, 1, 1, '2025-04-01 10:00:00', '2025-04-01 12:00:00', 20.0, 2.0, 40.0, 20.0, 20.0), "
    //            "(2, 2, 2, '2025-04-02 09:00:00', '2025-04-02 11:00:00', 25.0, 2.0, 50.0, 25.0, 25.0), "
    //            "(3, 3, 3, '2025-04-03 14:00:00', '2025-04-03 16:00:00', 30.0, 2.0, 60.0, 30.0, 30.0)");

    qDebug() << "Database initialized and dummy data inserted successfully.";
    return true;
}
