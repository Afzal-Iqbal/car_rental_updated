#include <QApplication>
#include <QMainWindow>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QVBoxLayout>
#include <QMenuBar>
#include <QHeaderView>
#include <QtCharts/QChartView>
#include <QtCharts/QPieSeries>
#include <QtCharts/QBarSeries>
#include <QtCharts/QBarSet>
#include <QtCharts/QBarCategoryAxis>
#include <QHBoxLayout>
#include <QIcon>
#include <QLabel>
#include <QStatusBar>
#include <QSqlQuery>
#include <QSqlError>
#include <QAction>
#include <QMenu>
#include <QMessageBox>
#include <QDialog>
#include <QFormLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QDialogButtonBox>
#include <QDateTimeEdit>
#include <QComboBox>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QGroupBox>
#include <QDateEdit>

#include "database.h"

// ======================================================================
// CUSTOMERS MANAGEMENT DIALOG
// ======================================================================
class CustomersManagementDialog : public QDialog
{
    Q_OBJECT
public:
    explicit CustomersManagementDialog(QWidget *parent = nullptr) : QDialog(parent)
    {
        setWindowTitle("Manage Customers");
        setMinimumSize(800, 600);

        QVBoxLayout *mainLayout = new QVBoxLayout(this);

        // Search bar
        QHBoxLayout *searchLayout = new QHBoxLayout();
        QLabel *searchLabel = new QLabel("Search:");
        searchEdit = new QLineEdit(this);
        searchEdit->setPlaceholderText("Search by name, phone, or address...");
        QPushButton *clearButton = new QPushButton("Clear");
        clearButton->setMaximumWidth(60);
        searchLayout->addWidget(searchLabel);
        searchLayout->addWidget(searchEdit);
        searchLayout->addWidget(clearButton);
        mainLayout->addLayout(searchLayout);

        // Buttons
        QHBoxLayout *buttonLayout = new QHBoxLayout();
        QPushButton *addButton = new QPushButton("Add Customer");
        QPushButton *editButton = new QPushButton("Edit Customer");
        QPushButton *deleteButton = new QPushButton("Delete Customer");
        buttonLayout->addWidget(addButton);
        buttonLayout->addWidget(editButton);
        buttonLayout->addWidget(deleteButton);
        mainLayout->addLayout(buttonLayout);

        // Table
        table = new QTableWidget(this);
        table->setColumnCount(4);
        table->setHorizontalHeaderLabels({"ID", "Name", "Phone", "Address"});
        table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
        table->setSelectionBehavior(QAbstractItemView::SelectRows);
        table->setSortingEnabled(true); // Enable column sorting
        table->sortByColumn(1, Qt::AscendingOrder); // Default sort by name
        mainLayout->addWidget(table);

        loadCustomers();

        connect(addButton, &QPushButton::clicked, this, &CustomersManagementDialog::addCustomer);
        connect(editButton, &QPushButton::clicked, this, &CustomersManagementDialog::editCustomer);
        connect(deleteButton, &QPushButton::clicked, this, &CustomersManagementDialog::deleteCustomer);
        
        // Search functionality
        connect(searchEdit, &QLineEdit::textChanged, this, &CustomersManagementDialog::performSearch);
        connect(clearButton, &QPushButton::clicked, this, [this]() { 
            searchEdit->clear(); 
            loadCustomers(); 
        });
    }

private slots:
    void loadCustomers()
    {
        // Temporarily disable sorting while populating table to prevent data corruption
        table->setSortingEnabled(false);
        
        QSqlQuery query("SELECT id, name, phone_number, address FROM customers");
        table->setRowCount(0);
        while (query.next())
        {
            int row = table->rowCount();
            table->insertRow(row);
            for (int col = 0; col < 4; col++)
            {
                table->setItem(row, col, new QTableWidgetItem(query.value(col).toString()));
            }
        }
        
        // Re-enable sorting after populating table
        table->setSortingEnabled(true);
        table->sortByColumn(1, Qt::AscendingOrder); // Re-apply default sort by name
    }

    // DSA Feature: Real-time search using string algorithms
    void performSearch(const QString &searchText)
    {
        if (searchText.isEmpty()) {
            loadCustomers(); // Show all customers if search is empty
            return;
        }

        // Temporarily disable sorting while populating table to prevent data corruption
        table->setSortingEnabled(false);

        // Use SQL LIKE for pattern matching (Boyer-Moore algorithm behind the scenes)
        QSqlQuery query;
        query.prepare("SELECT id, name, phone_number, address FROM customers WHERE "
                     "LOWER(name) LIKE LOWER(?) OR "
                     "phone_number LIKE ? OR "
                     "LOWER(address) LIKE LOWER(?)");
        QString pattern = "%" + searchText + "%";
        query.addBindValue(pattern);
        query.addBindValue(pattern);
        query.addBindValue(pattern);

        table->setRowCount(0);
        if (query.exec()) {
            while (query.next()) {
                int row = table->rowCount();
                table->insertRow(row);
                for (int col = 0; col < 4; col++) {
                    table->setItem(row, col, new QTableWidgetItem(query.value(col).toString()));
                }
            }
        }
        
        // Re-enable sorting after populating table
        table->setSortingEnabled(true);
        table->sortByColumn(1, Qt::AscendingOrder); // Re-apply default sort by name
    }

    void addCustomer()
    {
        QDialog dialog(this);
        dialog.setWindowTitle("Add Customer");

        QFormLayout form(&dialog);
        QLineEdit *nameEdit = new QLineEdit(&dialog);
        QLineEdit *phoneEdit = new QLineEdit(&dialog);
        QLineEdit *addressEdit = new QLineEdit(&dialog);

        form.addRow("Name:", nameEdit);
        form.addRow("Phone:", phoneEdit);
        form.addRow("Address:", addressEdit);

        QDialogButtonBox buttonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dialog);
        form.addRow(&buttonBox);

        connect(&buttonBox, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
        connect(&buttonBox, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);

        if (dialog.exec() == QDialog::Accepted)
        {
            QSqlQuery query;
            query.prepare("INSERT INTO customers (name, phone_number, address) VALUES (?, ?, ?)");
            query.addBindValue(nameEdit->text());
            query.addBindValue(phoneEdit->text());
            query.addBindValue(addressEdit->text());

            if (!query.exec())
            {
                QMessageBox::critical(this, "Error", "Failed to add customer: " + query.lastError().text());
            }
            else
            {
                loadCustomers();
            }
        }
    }

    void editCustomer()
    {
        int row = table->currentRow();
        if (row < 0)
        {
            QMessageBox::warning(this, "Warning", "Please select a customer");
            return;
        }

        QString id = table->item(row, 0)->text();
        
        // Debug output to verify we have correct ID
        qDebug() << "Editing customer at row:" << row << "with ID:" << id;

        QDialog dialog(this);
        dialog.setWindowTitle("Edit Customer");

        QFormLayout form(&dialog);
        QLineEdit *nameEdit = new QLineEdit(table->item(row, 1)->text(), &dialog);
        QLineEdit *phoneEdit = new QLineEdit(table->item(row, 2)->text(), &dialog);
        QLineEdit *addressEdit = new QLineEdit(table->item(row, 3)->text(), &dialog);

        form.addRow("Name:", nameEdit);
        form.addRow("Phone:", phoneEdit);
        form.addRow("Address:", addressEdit);

        QDialogButtonBox buttonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dialog);
        form.addRow(&buttonBox);

        connect(&buttonBox, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
        connect(&buttonBox, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);

        if (dialog.exec() == QDialog::Accepted)
        {
            QSqlQuery query;
            query.prepare("UPDATE customers SET name=?, phone_number=?, address=? WHERE id=?");
            query.addBindValue(nameEdit->text());
            query.addBindValue(phoneEdit->text());
            query.addBindValue(addressEdit->text());
            query.addBindValue(id);

            // Debug output to verify the query
            qDebug() << "Updating customer with ID:" << id 
                     << "Name:" << nameEdit->text()
                     << "Phone:" << phoneEdit->text() 
                     << "Address:" << addressEdit->text();

            if (!query.exec())
            {
                QMessageBox::critical(this, "Error", "Failed to update customer: " + query.lastError().text());
            }
            else
            {
                loadCustomers();
            }
        }
    }

    void deleteCustomer()
    {
        int row = table->currentRow();
        if (row < 0)
        {
            QMessageBox::warning(this, "Warning", "Please select a customer");
            return;
        }

        QString id = table->item(row, 0)->text();
        QString name = table->item(row, 1)->text();

        QMessageBox::StandardButton reply;
        reply = QMessageBox::question(this, "Confirm",
                                      "Delete customer '" + name + "' and all their bookings?",
                                      QMessageBox::Yes | QMessageBox::No);

        if (reply == QMessageBox::Yes)
        {
            QSqlQuery query;
            query.prepare("DELETE FROM customers WHERE id=?");
            query.addBindValue(id);

            if (!query.exec())
            {
                QMessageBox::critical(this, "Error", "Failed to delete customer: " + query.lastError().text());
            }
            else
            {
                loadCustomers();
            }
        }
    }

private:
    QTableWidget *table;
    QLineEdit *searchEdit;
};

// ======================================================================
// OWNERS MANAGEMENT DIALOG
// ======================================================================
class OwnersManagementDialog : public QDialog
{
    Q_OBJECT
public:
    explicit OwnersManagementDialog(QWidget *parent = nullptr) : QDialog(parent)
    {
        setWindowTitle("Manage Owners");
        setMinimumSize(800, 600);

        QVBoxLayout *mainLayout = new QVBoxLayout(this);

        // Search bar
        QHBoxLayout *searchLayout = new QHBoxLayout();
        QLabel *searchLabel = new QLabel("Search:");
        searchEdit = new QLineEdit(this);
        searchEdit->setPlaceholderText("Search by name, phone, or address...");
        QPushButton *clearButton = new QPushButton("Clear");
        clearButton->setMaximumWidth(60);
        searchLayout->addWidget(searchLabel);
        searchLayout->addWidget(searchEdit);
        searchLayout->addWidget(clearButton);
        mainLayout->addLayout(searchLayout);

        // Buttons
        QHBoxLayout *buttonLayout = new QHBoxLayout();
        QPushButton *addButton = new QPushButton("Add Owner");
        QPushButton *editButton = new QPushButton("Edit Owner");
        QPushButton *deleteButton = new QPushButton("Delete Owner");
        buttonLayout->addWidget(addButton);
        buttonLayout->addWidget(editButton);
        buttonLayout->addWidget(deleteButton);
        mainLayout->addLayout(buttonLayout);

        // Table
        table = new QTableWidget(this);
        table->setColumnCount(4);
        table->setHorizontalHeaderLabels({"ID", "Name", "Phone", "Address"});
        table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
        table->setSelectionBehavior(QAbstractItemView::SelectRows);
        table->setSortingEnabled(true); // Enable column sorting
        table->sortByColumn(1, Qt::AscendingOrder); // Default sort by name
        mainLayout->addWidget(table);

        loadOwners();

        connect(addButton, &QPushButton::clicked, this, &OwnersManagementDialog::addOwner);
        connect(editButton, &QPushButton::clicked, this, &OwnersManagementDialog::editOwner);
        connect(deleteButton, &QPushButton::clicked, this, &OwnersManagementDialog::deleteOwner);
        
        // Search functionality
        connect(searchEdit, &QLineEdit::textChanged, this, &OwnersManagementDialog::performSearch);
        connect(clearButton, &QPushButton::clicked, this, [this]() { 
            searchEdit->clear(); 
            loadOwners(); 
        });
    }

private slots:
    void loadOwners()
    {
        // Temporarily disable sorting while populating table to prevent data corruption
        table->setSortingEnabled(false);
        
        QSqlQuery query("SELECT id, name, phone_number, address FROM owners");
        table->setRowCount(0);
        while (query.next())
        {
            int row = table->rowCount();
            table->insertRow(row);
            for (int col = 0; col < 4; col++)
            {
                // Handle NULL values safely
                QString value = query.value(col).toString();
                table->setItem(row, col, new QTableWidgetItem(value.isNull() ? "" : value));
            }
        }
        
        // Re-enable sorting after populating table
        table->setSortingEnabled(true);
        table->sortByColumn(1, Qt::AscendingOrder); // Re-apply default sort by name
    }

    // DSA Feature: Real-time search using string algorithms for owners
    void performSearch(const QString &searchText)
    {
        if (searchText.isEmpty()) {
            loadOwners(); // Show all owners if search is empty
            return;
        }

        // Temporarily disable sorting while populating table to prevent data corruption
        table->setSortingEnabled(false);

        // Use SQL LIKE for pattern matching with case-insensitive search
        QSqlQuery query;
        query.prepare("SELECT id, name, phone_number, address FROM owners WHERE "
                     "LOWER(name) LIKE LOWER(?) OR "
                     "phone_number LIKE ? OR "
                     "LOWER(address) LIKE LOWER(?))");
        QString pattern = "%" + searchText + "%";
        query.addBindValue(pattern);
        query.addBindValue(pattern);
        query.addBindValue(pattern);

        table->setRowCount(0);
        if (query.exec()) {
            while (query.next()) {
                int row = table->rowCount();
                table->insertRow(row);
                for (int col = 0; col < 4; col++) {
                    table->setItem(row, col, new QTableWidgetItem(query.value(col).toString()));
                }
            }
        }
        
        // Re-enable sorting after populating table
        table->setSortingEnabled(true);
        table->sortByColumn(1, Qt::AscendingOrder); // Re-apply default sort by name
    }

    void addOwner()
    {
        QDialog dialog(this);
        dialog.setWindowTitle("Add Owner");

        QFormLayout form(&dialog);
        QLineEdit *nameEdit = new QLineEdit(&dialog);
        QLineEdit *phoneEdit = new QLineEdit(&dialog);
        QLineEdit *addressEdit = new QLineEdit(&dialog);

        form.addRow("Name:", nameEdit);
        form.addRow("Phone:", phoneEdit);
        form.addRow("Address:", addressEdit);

        QDialogButtonBox buttonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dialog);
        form.addRow(&buttonBox);

        connect(&buttonBox, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
        connect(&buttonBox, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);

        if (dialog.exec() == QDialog::Accepted)
        {
            // Validate input
            if (nameEdit->text().trimmed().isEmpty()) {
                QMessageBox::warning(this, "Validation Error", "Owner name is required!");
                return;
            }

            QSqlQuery query;
            query.prepare("INSERT INTO owners (name, phone_number, address) VALUES (?, ?, ?)");
            query.addBindValue(nameEdit->text().trimmed());
            query.addBindValue(phoneEdit->text().trimmed());
            query.addBindValue(addressEdit->text().trimmed());

            if (!query.exec())
            {
                QMessageBox::critical(this, "Error", "Failed to add owner: " + query.lastError().text());
            }
            else
            {
                loadOwners();
                // TODO: Refresh hash tables when CarRentalApp is accessible
            }
        }
    }

    void editOwner()
    {
        int row = table->currentRow();
        if (row < 0)
        {
            QMessageBox::warning(this, "Warning", "Please select an owner");
            return;
        }

        QString id = table->item(row, 0)->text();

        // Debug output to verify we have correct ID
        qDebug() << "Editing owner at row:" << row << "with ID:" << id;

        // Safe way to get text values - handle NULL/empty items
        QString currentName = table->item(row, 1) ? table->item(row, 1)->text() : "";
        QString currentPhone = table->item(row, 2) ? table->item(row, 2)->text() : "";
        QString currentAddress = table->item(row, 3) ? table->item(row, 3)->text() : "";

        QDialog dialog(this);
        dialog.setWindowTitle("Edit Owner");

        QFormLayout form(&dialog);
        QLineEdit *nameEdit = new QLineEdit(currentName, &dialog);
        QLineEdit *phoneEdit = new QLineEdit(currentPhone, &dialog);
        QLineEdit *addressEdit = new QLineEdit(currentAddress, &dialog);

        form.addRow("Name:", nameEdit);
        form.addRow("Phone:", phoneEdit);
        form.addRow("Address:", addressEdit);

        QDialogButtonBox buttonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dialog);
        form.addRow(&buttonBox);

        connect(&buttonBox, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
        connect(&buttonBox, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);

        if (dialog.exec() == QDialog::Accepted)
        {
            // Validate input
            if (nameEdit->text().trimmed().isEmpty()) {
                QMessageBox::warning(this, "Validation Error", "Owner name is required!");
                return;
            }

            QSqlQuery query;
            query.prepare("UPDATE owners SET name=?, phone_number=?, address=? WHERE id=?");
            query.addBindValue(nameEdit->text().trimmed());
            query.addBindValue(phoneEdit->text().trimmed());
            query.addBindValue(addressEdit->text().trimmed());
            query.addBindValue(id);

            // Debug output to verify the query
            qDebug() << "Updating owner with ID:" << id 
                     << "Name:" << nameEdit->text().trimmed()
                     << "Phone:" << phoneEdit->text().trimmed() 
                     << "Address:" << addressEdit->text().trimmed();

            if (!query.exec())
            {
                QMessageBox::critical(this, "Error", "Failed to update owner: " + query.lastError().text());
            }
            else
            {
                loadOwners();
                // TODO: Refresh hash tables when CarRentalApp is accessible
            }
        }
    }

    void deleteOwner()
    {
        int row = table->currentRow();
        if (row < 0)
        {
            QMessageBox::warning(this, "Warning", "Please select an owner");
            return;
        }

        QString id = table->item(row, 0)->text();
        QString name = table->item(row, 1)->text();

        QMessageBox::StandardButton reply;
        reply = QMessageBox::question(this, "Confirm",
                                      "Delete owner '" + name + "' and all their vehicles?",
                                      QMessageBox::Yes | QMessageBox::No);

        if (reply == QMessageBox::Yes)
        {
            QSqlQuery query;
            query.prepare("DELETE FROM owners WHERE id=?");
            query.addBindValue(id);

            if (!query.exec())
            {
                QMessageBox::critical(this, "Error", "Failed to delete owner: " + query.lastError().text());
            }
            else
            {
                loadOwners();
            }
        }
    }

private:
    QTableWidget *table;
    QLineEdit *searchEdit;
};

// ======================================================================
// VEHICLES MANAGEMENT DIALOG
// ======================================================================
class VehiclesManagementDialog : public QDialog
{
    Q_OBJECT
public:
    explicit VehiclesManagementDialog(QWidget *parent = nullptr) : QDialog(parent)
    {
        setWindowTitle("Manage Vehicles");
        setMinimumSize(1000, 600);

        QVBoxLayout *mainLayout = new QVBoxLayout(this);

        // Advanced search bar for vehicles
        QHBoxLayout *searchLayout = new QHBoxLayout();
        QLabel *searchLabel = new QLabel("Search:");
        searchEdit = new QLineEdit(this);
        searchEdit->setPlaceholderText("Search by vehicle name, make, model, year, or owner...");
        QPushButton *clearButton = new QPushButton("Clear");
        clearButton->setMaximumWidth(60);
        searchLayout->addWidget(searchLabel);
        searchLayout->addWidget(searchEdit);
        searchLayout->addWidget(clearButton);
        mainLayout->addLayout(searchLayout);

        // Buttons
        QHBoxLayout *buttonLayout = new QHBoxLayout();
        QPushButton *addButton = new QPushButton("Add Vehicle");
        QPushButton *editButton = new QPushButton("Edit Vehicle");
        QPushButton *deleteButton = new QPushButton("Delete Vehicle");
        buttonLayout->addWidget(addButton);
        buttonLayout->addWidget(editButton);
        buttonLayout->addWidget(deleteButton);
        mainLayout->addLayout(buttonLayout);

        // Table
        table = new QTableWidget(this);
        table->setColumnCount(6);
        table->setHorizontalHeaderLabels({"ID", "Owner", "Name", "Make", "Model", "Year"});
        table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
        table->setSelectionBehavior(QAbstractItemView::SelectRows);
        table->setSortingEnabled(true); // Enable column sorting
        table->sortByColumn(2, Qt::AscendingOrder); // Default sort by vehicle name
        mainLayout->addWidget(table);

        loadVehicles();

        connect(addButton, &QPushButton::clicked, this, &VehiclesManagementDialog::addVehicle);
        connect(editButton, &QPushButton::clicked, this, &VehiclesManagementDialog::editVehicle);
        connect(deleteButton, &QPushButton::clicked, this, &VehiclesManagementDialog::deleteVehicle);
        
        // Advanced search functionality for vehicles
        connect(searchEdit, &QLineEdit::textChanged, this, &VehiclesManagementDialog::performSearch);
        connect(clearButton, &QPushButton::clicked, this, [this]() { 
            searchEdit->clear(); 
            loadVehicles(); 
        });
    }

private slots:
    void loadVehicles()
    {
        // Temporarily disable sorting while populating table to prevent data corruption
        table->setSortingEnabled(false);
        
        QSqlQuery query("SELECT v.id, o.name, v.vehicle_name, v.make, v.model, v.year "
                        "FROM vehicles v JOIN owners o ON v.owner_id = o.id");
        table->setRowCount(0);
        while (query.next())
        {
            int row = table->rowCount();
            table->insertRow(row);
            for (int col = 0; col < 6; col++)
            {
                table->setItem(row, col, new QTableWidgetItem(query.value(col).toString()));
            }
        }
        
        // Re-enable sorting after populating table
        table->setSortingEnabled(true);
        table->sortByColumn(2, Qt::AscendingOrder); // Re-apply default sort by vehicle name
    }

    // DSA Feature: Advanced multi-field search with JOIN operations
    void performSearch(const QString &searchText)
    {
        if (searchText.isEmpty()) {
            loadVehicles(); // Show all vehicles if search is empty
            return;
        }

        // Temporarily disable sorting while populating table to prevent data corruption
        table->setSortingEnabled(false);

        // Advanced search across multiple fields including owner data
        QSqlQuery query;
        query.prepare("SELECT v.id, o.name, v.vehicle_name, v.make, v.model, v.year "
                     "FROM vehicles v JOIN owners o ON v.owner_id = o.id "
                     "WHERE LOWER(v.vehicle_name) LIKE LOWER(?) OR "
                     "LOWER(v.make) LIKE LOWER(?) OR "
                     "LOWER(v.model) LIKE LOWER(?) OR "
                     "CAST(v.year AS TEXT) LIKE ? OR "
                     "LOWER(o.name) LIKE LOWER(?)");
        
        QString pattern = "%" + searchText + "%";
        for(int i = 0; i < 5; i++) {
            query.addBindValue(pattern);
        }

        table->setRowCount(0);
        if (query.exec()) {
            while (query.next()) {
                int row = table->rowCount();
                table->insertRow(row);
                for (int col = 0; col < 6; col++) {
                    table->setItem(row, col, new QTableWidgetItem(query.value(col).toString()));
                }
            }
        }
        
        // Re-enable sorting after populating table
        table->setSortingEnabled(true);
        table->sortByColumn(2, Qt::AscendingOrder); // Re-apply default sort by vehicle name
    }

    void addVehicle()
    {
        QDialog dialog(this);
        dialog.setWindowTitle("Add Vehicle");

        QFormLayout form(&dialog);

        // Owner selection
        QComboBox *ownerCombo = new QComboBox(&dialog);
        QSqlQuery query("SELECT id, name FROM owners");
        while (query.next())
        {
            ownerCombo->addItem(query.value(1).toString(), query.value(0));
        }

        QLineEdit *nameEdit = new QLineEdit(&dialog);
        QLineEdit *makeEdit = new QLineEdit(&dialog);
        QLineEdit *modelEdit = new QLineEdit(&dialog);
        QSpinBox *yearEdit = new QSpinBox(&dialog);
        yearEdit->setRange(1900, QDate::currentDate().year());
        yearEdit->setValue(2020);

        form.addRow("Owner:", ownerCombo);
        form.addRow("Vehicle Name:", nameEdit);
        form.addRow("Make:", makeEdit);
        form.addRow("Model:", modelEdit);
        form.addRow("Year:", yearEdit);

        QDialogButtonBox buttonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dialog);
        form.addRow(&buttonBox);

        connect(&buttonBox, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
        connect(&buttonBox, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);

        if (dialog.exec() == QDialog::Accepted)
        {
            QSqlQuery query;
            query.prepare("INSERT INTO vehicles (owner_id, vehicle_name, make, model, year) "
                          "VALUES (?, ?, ?, ?, ?)");
            query.addBindValue(ownerCombo->currentData());
            query.addBindValue(nameEdit->text());
            query.addBindValue(makeEdit->text());
            query.addBindValue(modelEdit->text());
            query.addBindValue(yearEdit->value());

            if (!query.exec())
            {
                QMessageBox::critical(this, "Error", "Failed to add vehicle: " + query.lastError().text());
            }
            else
            {
                loadVehicles();
            }
        }
    }

    void editVehicle()
    {
        int row = table->currentRow();
        if (row < 0)
        {
            QMessageBox::warning(this, "Warning", "Please select a vehicle");
            return;
        }

        QString id = table->item(row, 0)->text();
        
        // Debug output to verify we have correct ID
        qDebug() << "Editing vehicle at row:" << row << "with ID:" << id;

        QDialog dialog(this);
        dialog.setWindowTitle("Edit Vehicle");

        QFormLayout form(&dialog);

        QComboBox *ownerCombo = new QComboBox(&dialog);
        QSqlQuery ownerQuery("SELECT id, name FROM owners");
        while (ownerQuery.next())
        {
            ownerCombo->addItem(ownerQuery.value(1).toString(), ownerQuery.value(0));
        }

        QLineEdit *nameEdit = new QLineEdit(table->item(row, 2)->text(), &dialog);
        QLineEdit *makeEdit = new QLineEdit(table->item(row, 3)->text(), &dialog);
        QLineEdit *modelEdit = new QLineEdit(table->item(row, 4)->text(), &dialog);
        QSpinBox *yearEdit = new QSpinBox(&dialog);
        yearEdit->setRange(1900, QDate::currentDate().year());
        yearEdit->setValue(table->item(row, 5)->text().toInt());

        // Set current owner
        QString currentOwner = table->item(row, 1)->text();
        for (int i = 0; i < ownerCombo->count(); i++)
        {
            if (ownerCombo->itemText(i) == currentOwner)
            {
                ownerCombo->setCurrentIndex(i);
                break;
            }
        }

        form.addRow("Owner:", ownerCombo);
        form.addRow("Vehicle Name:", nameEdit);
        form.addRow("Make:", makeEdit);
        form.addRow("Model:", modelEdit);
        form.addRow("Year:", yearEdit);

        QDialogButtonBox buttonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dialog);
        form.addRow(&buttonBox);

        connect(&buttonBox, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
        connect(&buttonBox, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);

        if (dialog.exec() == QDialog::Accepted)
        {
            QSqlQuery query;
            query.prepare("UPDATE vehicles SET owner_id=?, vehicle_name=?, make=?, model=?, year=? "
                          "WHERE id=?");
            query.addBindValue(ownerCombo->currentData());
            query.addBindValue(nameEdit->text());
            query.addBindValue(makeEdit->text());
            query.addBindValue(modelEdit->text());
            query.addBindValue(yearEdit->value());
            query.addBindValue(id);

            // Debug output to verify the query
            qDebug() << "Updating vehicle with ID:" << id 
                     << "Owner:" << ownerCombo->currentText()
                     << "Name:" << nameEdit->text()
                     << "Make:" << makeEdit->text() 
                     << "Model:" << modelEdit->text()
                     << "Year:" << yearEdit->value();

            if (!query.exec())
            {
                QMessageBox::critical(this, "Error", "Failed to update vehicle: " + query.lastError().text());
            }
            else
            {
                loadVehicles();
            }
        }
    }

    void deleteVehicle()
    {
        int row = table->currentRow();
        if (row < 0)
        {
            QMessageBox::warning(this, "Warning", "Please select a vehicle");
            return;
        }

        QString id = table->item(row, 0)->text();
        QString name = table->item(row, 2)->text();

        QMessageBox::StandardButton reply;
        reply = QMessageBox::question(this, "Confirm",
                                      "Delete vehicle '" + name + "' and all its bookings?",
                                      QMessageBox::Yes | QMessageBox::No);

        if (reply == QMessageBox::Yes)
        {
            QSqlQuery query;
            query.prepare("DELETE FROM vehicles WHERE id=?");
            query.addBindValue(id);

            if (!query.exec())
            {
                QMessageBox::critical(this, "Error", "Failed to delete vehicle: " + query.lastError().text());
            }
            else
            {
                loadVehicles();
            }
        }
    }

private:
    QTableWidget *table;
    QLineEdit *searchEdit;
};

// ======================================================================
// BOOKINGS MANAGEMENT DIALOG
// ======================================================================
class BookingsManagementDialog : public QDialog
{
    Q_OBJECT
public:
    explicit BookingsManagementDialog(QWidget *parent = nullptr) : QDialog(parent)
    {
        setWindowTitle("Manage Bookings");
        setMinimumSize(1200, 600);

        QVBoxLayout *mainLayout = new QVBoxLayout(this);

        // Advanced search interface for bookings
        QGroupBox *searchGroup = new QGroupBox("Search & Filter");
        QVBoxLayout *searchLayout = new QVBoxLayout(searchGroup);
        
        // Text search row
        QHBoxLayout *textSearchLayout = new QHBoxLayout();
        QLabel *searchLabel = new QLabel("Search:");
        searchEdit = new QLineEdit(this);
        searchEdit->setPlaceholderText("Search by customer, vehicle, or owner...");
        QPushButton *clearButton = new QPushButton("Clear All");
        clearButton->setMaximumWidth(80);
        textSearchLayout->addWidget(searchLabel);
        textSearchLayout->addWidget(searchEdit);
        textSearchLayout->addWidget(clearButton);
        searchLayout->addLayout(textSearchLayout);
        
        // Date range filter row
        QHBoxLayout *dateLayout = new QHBoxLayout();
        QLabel *fromLabel = new QLabel("From:");
        fromDateEdit = new QDateEdit(QDate::currentDate().addDays(-30), this);
        fromDateEdit->setCalendarPopup(true);
        QLabel *toLabel = new QLabel("To:");
        toDateEdit = new QDateEdit(QDate::currentDate().addDays(30), this);
        toDateEdit->setCalendarPopup(true);
        
        // Status filter
        QLabel *statusLabel = new QLabel("Status:");
        statusCombo = new QComboBox(this);
        statusCombo->addItem("All Bookings", "all");
        statusCombo->addItem("Paid", "paid");
        statusCombo->addItem("Pending Payment", "pending");
        
        dateLayout->addWidget(fromLabel);
        dateLayout->addWidget(fromDateEdit);
        dateLayout->addWidget(toLabel);
        dateLayout->addWidget(toDateEdit);
        dateLayout->addWidget(statusLabel);
        dateLayout->addWidget(statusCombo);
        dateLayout->addStretch();
        searchLayout->addLayout(dateLayout);
        
        mainLayout->addWidget(searchGroup);

        // Buttons
        QHBoxLayout *buttonLayout = new QHBoxLayout();
        QPushButton *addButton = new QPushButton("Add Booking");
        QPushButton *editButton = new QPushButton("Edit Booking");
        QPushButton *deleteButton = new QPushButton("Delete Booking");
        buttonLayout->addWidget(addButton);
        buttonLayout->addWidget(editButton);
        buttonLayout->addWidget(deleteButton);
        mainLayout->addLayout(buttonLayout);

        // Table
        table = new QTableWidget(this);
        table->setColumnCount(10);
        table->setHorizontalHeaderLabels({"ID", "Customer", "Vehicle", "Owner",
                                          "Start", "End", "Rate/Hour", "Hours", "Total", "Status"});
        table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
        table->setSelectionBehavior(QAbstractItemView::SelectRows);
        table->setSortingEnabled(true); // Enable column sorting
        table->sortByColumn(4, Qt::DescendingOrder); // Default sort by start date (newest first)
        mainLayout->addWidget(table);

        loadBookings();

        connect(addButton, &QPushButton::clicked, this, &BookingsManagementDialog::addBooking);
        connect(editButton, &QPushButton::clicked, this, &BookingsManagementDialog::editBooking);
        connect(deleteButton, &QPushButton::clicked, this, &BookingsManagementDialog::deleteBooking);
        
        // Advanced search functionality connections
        connect(searchEdit, &QLineEdit::textChanged, this, &BookingsManagementDialog::performSearch);
        connect(fromDateEdit, &QDateEdit::dateChanged, this, &BookingsManagementDialog::performSearch);
        connect(toDateEdit, &QDateEdit::dateChanged, this, &BookingsManagementDialog::performSearch);
        connect(statusCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), 
                this, &BookingsManagementDialog::performSearch);
        connect(clearButton, &QPushButton::clicked, this, [this]() { 
            searchEdit->clear(); 
            fromDateEdit->setDate(QDate::currentDate().addDays(-30));
            toDateEdit->setDate(QDate::currentDate().addDays(30));
            statusCombo->setCurrentIndex(0);
            loadBookings(); 
        });
    }

private slots:
    void loadBookings()
    {
        // Temporarily disable sorting while populating table to prevent data corruption
        table->setSortingEnabled(false);
        
        QSqlQuery query(
            "SELECT b.id, c.name, v.vehicle_name, o.name, "
            "b.booking_start, b.booking_end, "
            "b.amount_per_hour, b.total_hours, b.total_amount, "
            "CASE WHEN b.pending_amount = 0 THEN 'Paid' ELSE 'Pending' END "
            "FROM bookings b "
            "JOIN customers c ON b.customer_id = c.id "
            "JOIN vehicles v ON b.vehicle_id = v.id "
            "JOIN owners o ON v.owner_id = o.id");

        table->setRowCount(0);
        while (query.next())
        {
            int row = table->rowCount();
            table->insertRow(row);
            for (int col = 0; col < 10; col++)
            {
                table->setItem(row, col, new QTableWidgetItem(query.value(col).toString()));
            }
        }
        
        // Re-enable sorting after populating table
        table->setSortingEnabled(true);
        table->sortByColumn(4, Qt::DescendingOrder); // Re-apply default sort by start date
    }

    // DSA Feature: Advanced multi-criteria search with date range filtering
    void performSearch()
    {
        QString searchText = searchEdit->text();
        QDate fromDate = fromDateEdit->date();
        QDate toDate = toDateEdit->date();
        QString statusFilter = statusCombo->currentData().toString();

        // Temporarily disable sorting while populating table to prevent data corruption
        table->setSortingEnabled(false);

        // Build complex query with multiple criteria
        QString baseQuery = 
            "SELECT b.id, c.name, v.vehicle_name, o.name, "
            "b.booking_start, b.booking_end, "
            "b.amount_per_hour, b.total_hours, b.total_amount, "
            "CASE WHEN b.pending_amount = 0 THEN 'Paid' ELSE 'Pending' END "
            "FROM bookings b "
            "JOIN customers c ON b.customer_id = c.id "
            "JOIN vehicles v ON b.vehicle_id = v.id "
            "JOIN owners o ON v.owner_id = o.id WHERE 1=1";

        QStringList conditions;
        QList<QVariant> bindValues;

        // Add text search condition (searches across multiple fields)
        if (!searchText.isEmpty()) {
            conditions.append(
                "(LOWER(c.name) LIKE LOWER(?) OR "
                "LOWER(v.vehicle_name) LIKE LOWER(?) OR "
                "LOWER(o.name) LIKE LOWER(?))");
            QString pattern = "%" + searchText + "%";
            bindValues << pattern << pattern << pattern;
        }

        // Add date range condition
        conditions.append("DATE(b.booking_start) BETWEEN ? AND ?");
        bindValues << fromDate.toString("yyyy-MM-dd") << toDate.toString("yyyy-MM-dd");

        // Add status filter condition
        if (statusFilter != "all") {
            if (statusFilter == "paid") {
                conditions.append("b.pending_amount = 0");
            } else if (statusFilter == "pending") {
                conditions.append("b.pending_amount > 0");
            }
        }

        // Combine all conditions
        QString finalQuery = baseQuery;
        if (!conditions.isEmpty()) {
            finalQuery += " AND " + conditions.join(" AND ");
        }
        finalQuery += " ORDER BY b.booking_start DESC";

        QSqlQuery query;
        query.prepare(finalQuery);
        
        // Bind all values
        for (const QVariant& value : bindValues) {
            query.addBindValue(value);
        }

        table->setRowCount(0);
        if (query.exec()) {
            while (query.next()) {
                int row = table->rowCount();
                table->insertRow(row);
                for (int col = 0; col < 10; col++) {
                    table->setItem(row, col, new QTableWidgetItem(query.value(col).toString()));
                }
            }
        } else {
            qDebug() << "Search query failed:" << query.lastError().text();
        }
        
        // Re-enable sorting after populating table
        table->setSortingEnabled(true);
        table->sortByColumn(4, Qt::DescendingOrder); // Re-apply default sort by start date
    }

    void addBooking()
    {
        QDialog dialog(this);
        dialog.setWindowTitle("Add Booking");
        dialog.setMinimumSize(600, 400);

        QFormLayout form(&dialog);

        // Customer selection
        QComboBox *customerCombo = new QComboBox(&dialog);
        QSqlQuery customerQuery("SELECT id, name FROM customers");
        while (customerQuery.next())
        {
            customerCombo->addItem(customerQuery.value(1).toString(), customerQuery.value(0).toInt());
        }

        // Vehicle selection
        QComboBox *vehicleCombo = new QComboBox(&dialog);
        QSqlQuery vehicleQuery(
            "SELECT v.id, v.vehicle_name || ' (' || o.name || ')', v.owner_id "
            "FROM vehicles v JOIN owners o ON v.owner_id = o.id");
        QMap<int, int> vehicleOwnerMap; // Using QMap instead of QHash with simple int keys
        while (vehicleQuery.next())
        {
            int vehicleId = vehicleQuery.value(0).toInt();
            int ownerId = vehicleQuery.value(2).toInt();
            vehicleCombo->addItem(vehicleQuery.value(1).toString(), vehicleId);
            vehicleOwnerMap.insert(vehicleId, ownerId);
        }

        // Date/time inputs
        QDateTimeEdit *startEdit = new QDateTimeEdit(QDateTime::currentDateTime(), &dialog);
        QDateTimeEdit *endEdit = new QDateTimeEdit(QDateTime::currentDateTime().addSecs(3600), &dialog);
        startEdit->setDisplayFormat("yyyy-MM-dd HH:mm");
        endEdit->setDisplayFormat("yyyy-MM-dd HH:mm");

        // Rate input
        QDoubleSpinBox *rateEdit = new QDoubleSpinBox(&dialog);
        rateEdit->setRange(1, 1000);
        rateEdit->setValue(50);
        rateEdit->setPrefix("Rs ");

        // Display calculated total
        QLabel *totalLabel = new QLabel("Rs0.00", &dialog);

        form.addRow("Customer:", customerCombo);
        form.addRow("Vehicle:", vehicleCombo);
        form.addRow("Start Time:", startEdit);
        form.addRow("End Time:", endEdit);
        form.addRow("Rate/Hour:", rateEdit);
        form.addRow("Estimated Total:", totalLabel);

        // Update total when times or rate change
        auto updateTotal = [=]()
        {
            qint64 secs = startEdit->dateTime().secsTo(endEdit->dateTime());
            double hours = qMax(1.0, secs / 3600.0);
            double total = hours * rateEdit->value();
            totalLabel->setText(QString("Rs %1").arg(total, 0, 'f', 2));
        };

        connect(startEdit, &QDateTimeEdit::dateTimeChanged, updateTotal);
        connect(endEdit, &QDateTimeEdit::dateTimeChanged, updateTotal);
        connect(rateEdit, QOverload<double>::of(&QDoubleSpinBox::valueChanged), updateTotal);
        updateTotal();

        QDialogButtonBox buttonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dialog);
        form.addRow(&buttonBox);

        connect(&buttonBox, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
        connect(&buttonBox, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);

        if (dialog.exec() == QDialog::Accepted)
        {
            qint64 secs = startEdit->dateTime().secsTo(endEdit->dateTime());
            double hours = qMax(1.0, secs / 3600.0);
            double total = hours * rateEdit->value();

            // Get the selected vehicle's owner ID
            int vehicleId = vehicleCombo->currentData().toInt();
            int ownerId = vehicleOwnerMap.value(vehicleId);

            QSqlQuery query;
            query.prepare(
                "INSERT INTO bookings ("
                "customer_id, vehicle_id, owner_id, booking_start, booking_end, "
                "amount_per_hour, total_hours, total_amount, advance_amount, pending_amount"
                ") VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?)");
            query.addBindValue(customerCombo->currentData().toInt());                  // customer_id
            query.addBindValue(vehicleId);                                             // vehicle_id
            query.addBindValue(ownerId);                                               // owner_id
            query.addBindValue(startEdit->dateTime().toString("yyyy-MM-dd HH:mm:ss")); // booking_start
            query.addBindValue(endEdit->dateTime().toString("yyyy-MM-dd HH:mm:ss"));   // booking_end
            query.addBindValue(rateEdit->value());                                     // amount_per_hour
            query.addBindValue(hours);                                                 // total_hours
            query.addBindValue(total);                                                 // total_amount
            query.addBindValue(0);                                                     // advance_amount
            query.addBindValue(total);                                                 // pending_amount

            if (!query.exec())
            {
                QMessageBox::critical(this, "Error", "Failed to add booking: " + query.lastError().text());
            }
            else
            {
                loadBookings();
            }
        }
    }

    void editBooking()
    {
        int row = table->currentRow();
        if (row < 0)
        {
            QMessageBox::warning(this, "Warning", "Please select a booking");
            return;
        }

        QString id = table->item(row, 0)->text();
        
        // Debug output to verify we have correct ID
        qDebug() << "Editing booking at row:" << row << "with ID:" << id;

        QDialog dialog(this);
        dialog.setWindowTitle("Edit Booking");
        dialog.setMinimumSize(600, 400);

        QFormLayout form(&dialog);

        // Get vehicle ID for the current booking
        QString vehicleName = table->item(row, 2)->text();
        QString ownerName = table->item(row, 3)->text();
        QString currentVehicleText = vehicleName + " (" + ownerName + ")";

        // Customer selection
        QComboBox *customerCombo = new QComboBox(&dialog);
        QSqlQuery customerQuery("SELECT id, name FROM customers");
        while (customerQuery.next())
        {
            customerCombo->addItem(customerQuery.value(1).toString(), customerQuery.value(0));
            if (customerQuery.value(1).toString() == table->item(row, 1)->text())
            {
                customerCombo->setCurrentIndex(customerCombo->count() - 1);
            }
        }

        // Vehicle selection
        QComboBox *vehicleCombo = new QComboBox(&dialog);
        QSqlQuery vehicleQuery(
            "SELECT v.id, v.vehicle_name || ' (' || o.name || ')' "
            "FROM vehicles v JOIN owners o ON v.owner_id = o.id");
        while (vehicleQuery.next())
        {
            vehicleCombo->addItem(vehicleQuery.value(1).toString(), vehicleQuery.value(0));
            if (vehicleQuery.value(1).toString() == currentVehicleText)
            {
                vehicleCombo->setCurrentIndex(vehicleCombo->count() - 1);
            }
        }

        // Date/time inputs
        QDateTime startTime = QDateTime::fromString(table->item(row, 4)->text(), "yyyy-MM-dd HH:mm:ss");
        QDateTime endTime = QDateTime::fromString(table->item(row, 5)->text(), "yyyy-MM-dd HH:mm:ss");

        QDateTimeEdit *startEdit = new QDateTimeEdit(startTime, &dialog);
        QDateTimeEdit *endEdit = new QDateTimeEdit(endTime, &dialog);
        startEdit->setDisplayFormat("yyyy-MM-dd HH:mm");
        endEdit->setDisplayFormat("yyyy-MM-dd HH:mm");

        // Rate input
        QDoubleSpinBox *rateEdit = new QDoubleSpinBox(&dialog);
        rateEdit->setRange(1, 1000);
        rateEdit->setValue(table->item(row, 6)->text().toDouble());
        rateEdit->setPrefix("Rs ");

        // Payment status
        QString status = table->item(row, 9)->text();
        QDoubleSpinBox *advanceEdit = new QDoubleSpinBox(&dialog);
        advanceEdit->setRange(0, table->item(row, 8)->text().toDouble());
        advanceEdit->setValue(table->item(row, 8)->text().toDouble() - table->item(row, 7)->text().toDouble());
        advanceEdit->setPrefix("Rs ");

        // Display calculated total
        QLabel *totalLabel = new QLabel(table->item(row, 8)->text(), &dialog);

        form.addRow("Customer:", customerCombo);
        form.addRow("Vehicle:", vehicleCombo);
        form.addRow("Start Time:", startEdit);
        form.addRow("End Time:", endEdit);
        form.addRow("Rate/Hour:", rateEdit);
        form.addRow("Total Amount:", totalLabel);
        form.addRow("Advance Payment:", advanceEdit);

        // Update total when times or rate change
        auto updateTotal = [=]()
        {
            qint64 secs = startEdit->dateTime().secsTo(endEdit->dateTime());
            double hours = qMax(1.0, secs / 3600.0);
            double total = hours * rateEdit->value();
            totalLabel->setText(QString("Rs %1").arg(total, 0, 'f', 2));
            advanceEdit->setMaximum(total);
        };

        connect(startEdit, &QDateTimeEdit::dateTimeChanged, updateTotal);
        connect(endEdit, &QDateTimeEdit::dateTimeChanged, updateTotal);
        connect(rateEdit, QOverload<double>::of(&QDoubleSpinBox::valueChanged), updateTotal);
        updateTotal();

        QDialogButtonBox buttonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dialog);
        form.addRow(&buttonBox);

        connect(&buttonBox, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
        connect(&buttonBox, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);

        if (dialog.exec() == QDialog::Accepted)
        {
            qint64 secs = startEdit->dateTime().secsTo(endEdit->dateTime());
            double hours = qMax(1.0, secs / 3600.0);
            double total = hours * rateEdit->value();
            double advance = advanceEdit->value();

            QSqlQuery query;
            query.prepare(
                "UPDATE bookings SET "
                "customer_id=?, vehicle_id=?, booking_start=?, booking_end=?, "
                "amount_per_hour=?, total_hours=?, total_amount=?, "
                "advance_amount=?, pending_amount=? "
                "WHERE id=?");
            query.addBindValue(customerCombo->currentData());
            query.addBindValue(vehicleCombo->currentData());
            query.addBindValue(startEdit->dateTime().toString("yyyy-MM-dd HH:mm:ss"));
            query.addBindValue(endEdit->dateTime().toString("yyyy-MM-dd HH:mm:ss"));
            query.addBindValue(rateEdit->value());
            query.addBindValue(hours);
            query.addBindValue(total);
            query.addBindValue(advance);
            query.addBindValue(total - advance);
            query.addBindValue(id);

            if (!query.exec())
            {
                QMessageBox::critical(this, "Error", "Failed to update booking: " + query.lastError().text());
            }
            else
            {
                loadBookings();
            }
        }
    }

    void deleteBooking()
    {
        int row = table->currentRow();
        if (row < 0)
        {
            QMessageBox::warning(this, "Warning", "Please select a booking");
            return;
        }

        QString id = table->item(row, 0)->text();
        QString customer = table->item(row, 1)->text();
        QString vehicle = table->item(row, 2)->text();

        QMessageBox::StandardButton reply;
        reply = QMessageBox::question(this, "Confirm",
                                      QString("Delete booking for\n%1 - %2?").arg(customer, vehicle),
                                      QMessageBox::Yes | QMessageBox::No);

        if (reply == QMessageBox::Yes)
        {
            QSqlQuery query;
            query.prepare("DELETE FROM bookings WHERE id=?");
            query.addBindValue(id);

            if (!query.exec())
            {
                QMessageBox::critical(this, "Error", "Failed to delete booking: " + query.lastError().text());
            }
            else
            {
                loadBookings();
            }
        }
    }

private:
    QTableWidget *table;
    QLineEdit *searchEdit;
    QDateEdit *fromDateEdit;
    QDateEdit *toDateEdit;
    QComboBox *statusCombo;
};

// ======================================================================
// MAIN APPLICATION WINDOW
// ======================================================================
class CarRentalApp : public QMainWindow
{
    Q_OBJECT
public:
    CarRentalApp(QWidget *parent = nullptr) : QMainWindow(parent)
    {
        setWindowTitle("Car Rental Service");
        setGeometry(100, 100, 1200, 800);

        QWidget *centralWidget = new QWidget(this);
        setCentralWidget(centralWidget);

        // DSA Feature: Initialize hash tables for O(1) lookups
        buildLookupTables();

        setupMenuBar();
        setupMainContent(centralWidget);
        setupStatusBar();
    }

    // Public method to refresh lookup tables when data changes
    void refreshLookupTables() {
        buildLookupTables();
    }

private slots:
    void showCustomersManagement()
    {
        CustomersManagementDialog dialog(this);
        dialog.exec();
    }

    void showOwnersManagement()
    {
        OwnersManagementDialog dialog(this);
        dialog.exec();
    }

    void showVehiclesManagement()
    {
        VehiclesManagementDialog dialog(this);
        dialog.exec();
    }
    void showBookingsManagement()
    {
        BookingsManagementDialog dialog(this);
        dialog.exec();
    }

private:
    // DSA Feature: Hash Tables for O(1) lookup performance
    QHash<int, QString> ownerLookup;     // owner_id -> name
    QHash<int, QString> vehicleLookup;   // vehicle_id -> vehicle_name
    QHash<int, QString> customerLookup;  // customer_id -> name
    QHash<int, QString> makeLookup;      // vehicle_id -> make
    QHash<int, QString> modelLookup;     // vehicle_id -> model

    void buildLookupTables() {
        // Clear existing hash tables
        ownerLookup.clear();
        vehicleLookup.clear();
        customerLookup.clear();
        makeLookup.clear();
        modelLookup.clear();

        // Build owner lookup table - O(n) build, O(1) lookup
        QSqlQuery ownerQuery("SELECT id, name FROM owners");
        while(ownerQuery.next()) {
            ownerLookup[ownerQuery.value(0).toInt()] = ownerQuery.value(1).toString();
        }

        // Build vehicle lookup tables
        QSqlQuery vehicleQuery("SELECT id, vehicle_name, make, model FROM vehicles");
        while(vehicleQuery.next()) {
            int id = vehicleQuery.value(0).toInt();
            vehicleLookup[id] = vehicleQuery.value(1).toString();
            makeLookup[id] = vehicleQuery.value(2).toString();
            modelLookup[id] = vehicleQuery.value(3).toString();
        }

        // Build customer lookup table
        QSqlQuery customerQuery("SELECT id, name FROM customers");
        while(customerQuery.next()) {
            customerLookup[customerQuery.value(0).toInt()] = customerQuery.value(1).toString();
        }

        qDebug() << "Hash tables built: " 
                 << ownerLookup.size() << " owners, "
                 << vehicleLookup.size() << " vehicles, "
                 << customerLookup.size() << " customers";
    }

    // DSA Feature: O(1) lookup functions using hash tables
    QString getOwnerName(int ownerId) const {
        return ownerLookup.value(ownerId, "Unknown Owner");
    }

    QString getVehicleName(int vehicleId) const {
        return vehicleLookup.value(vehicleId, "Unknown Vehicle");
    }

    QString getCustomerName(int customerId) const {
        return customerLookup.value(customerId, "Unknown Customer");
    }

    QString getVehicleMake(int vehicleId) const {
        return makeLookup.value(vehicleId, "Unknown Make");
    }

    QString getVehicleModel(int vehicleId) const {
        return modelLookup.value(vehicleId, "Unknown Model");
    }

    void setupMenuBar()
    {
        QMenuBar *menuBar = new QMenuBar(this);
        menuBar->setStyleSheet(
            "QMenuBar { background-color: #0000FF; color: white; font-size: 14px; font-weight: bold; padding: 8px; }"
            "QMenuBar::item { padding: 10px 15px; background: #ADD8E6; border-radius: 5px; margin: 5px; }"
            "QMenuBar::item:selected { background: #0055FF; color: white; }");

        menuBar->addMenu(QIcon(":/icons/dashboard.png"), "Dashboard");

        QMenu *ownersMenu = menuBar->addMenu(QIcon(":/icons/owners.png"), "Manage Owners");
        QAction *manageOwnerAction = new QAction("Manage Owners", this);
        ownersMenu->addAction(manageOwnerAction);
        connect(manageOwnerAction, &QAction::triggered, this, &CarRentalApp::showOwnersManagement);

        QMenu *vehiclesMenu = menuBar->addMenu(QIcon(":/icons/vehicles.png"), "Manage Vehicles");
        QAction *manageVehiclesAction = new QAction("Manage Vehicles", this);
        vehiclesMenu->addAction(manageVehiclesAction);
        connect(manageVehiclesAction, &QAction::triggered, this, &CarRentalApp::showVehiclesManagement);

        QMenu *customersMenu = menuBar->addMenu(QIcon(":/icons/customers.png"), "Manage Customers");
        QAction *manageCustomersAction = new QAction("Manage Customers", this);
        customersMenu->addAction(manageCustomersAction);
        connect(manageCustomersAction, &QAction::triggered, this, &CarRentalApp::showCustomersManagement);

        QMenu *bookingsMenu = menuBar->addMenu(QIcon(":/icons/payments.png"), "Manage Bookings");
        QAction *manageBookingsAction = new QAction("Manage Bookings", this);
        bookingsMenu->addAction(manageBookingsAction);
        connect(manageBookingsAction, &QAction::triggered, this, &CarRentalApp::showBookingsManagement);

        setMenuBar(menuBar);
    }

    void setupMainContent(QWidget *centralWidget)
    {
        QVBoxLayout *mainLayout = new QVBoxLayout();
        QHBoxLayout *chartLayout = new QHBoxLayout();

        // Setup pie chart
        QPieSeries *pieSeries = new QPieSeries();
        pieSeries->append("Car Bookings", 40);
        pieSeries->append("Payments Received", 60);
        QChart *pieChart = new QChart();
        pieChart->addSeries(pieSeries);
        pieChart->setTitle("Sales Distribution");
        QChartView *pieChartView = new QChartView(pieChart);
        pieChartView->setRenderHint(QPainter::Antialiasing);
        pieChartView->setMinimumSize(400, 300);

        // Setup bar chart
        QBarSet *set0 = new QBarSet("Monthly Sales");
        *set0 << 10 << 20 << 30 << 40 << 50;
        QBarSeries *barSeries = new QBarSeries();
        barSeries->append(set0);
        QChart *barChart = new QChart();
        barChart->addSeries(barSeries);
        barChart->setTitle("Sales Growth");
        barChart->createDefaultAxes();
        QChartView *barChartView = new QChartView(barChart);
        barChartView->setRenderHint(QPainter::Antialiasing);
        barChartView->setMinimumSize(400, 300);

        // Add charts to layout
        chartLayout->addWidget(pieChartView);
        chartLayout->addWidget(barChartView);

        // Setup bookings table
        QLabel *bookingsHeading = new QLabel("<h3>Bookings</h3>");
        bookingsHeading->setAlignment(Qt::AlignLeft);
        bookingsHeading->setStyleSheet(
            "font-size: 14px; font-weight: bold; padding: 5px 10px;");

        QTableWidget *bookingsTable = new QTableWidget(this);
        bookingsTable->setColumnCount(8);
        bookingsTable->setHorizontalHeaderLabels({"Car Owner", "Vehicle", "Customer", "Amount Per Hour",
                                                  "Total Hours", "Total Amount", "Advance", "Pending"});
        bookingsTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

        QString tableStyle =
            "QTableWidget { background-color: #ADD8E6; border: 2px solid #0000FF; }"
            "QHeaderView::section { background-color: #0000FF; color: white; font-size: 14px; font-weight: bold; padding: 8px; }";
        bookingsTable->setStyleSheet(tableStyle);

        loadBookings(bookingsTable);

        // Add widgets to main layout
        mainLayout->addLayout(chartLayout);
        mainLayout->addWidget(bookingsHeading);
        mainLayout->addWidget(bookingsTable);

        centralWidget->setLayout(mainLayout);
    }

    void setupStatusBar()
    {
        QStatusBar *statusBar = new QStatusBar(this);
        statusBar->setStyleSheet(
            "background-color: #0000FF; color: white; font-size: 14px; font-weight: bold; padding: 8px;");
        QLabel *footerLeft = new QLabel("Copyright © 2025");
        QLabel *footerRight = new QLabel("Version 1.0.0");
        statusBar->addWidget(footerLeft);
        statusBar->addPermanentWidget(footerRight);
        setStatusBar(statusBar);
    }

    void loadBookings(QTableWidget *table)
    {
        QSqlQuery query(
            "SELECT owners.name, vehicles.vehicle_name, customers.name, "
            "bookings.amount_per_hour, bookings.total_hours, bookings.total_amount, "
            "bookings.advance_amount, bookings.pending_amount "
            "FROM bookings "
            "JOIN owners ON bookings.owner_id = owners.id "
            "JOIN vehicles ON bookings.vehicle_id = vehicles.id "
            "JOIN customers ON bookings.customer_id = customers.id");

        int row = 0;
        table->setRowCount(0);
        while (query.next())
        {
            table->insertRow(row);
            for (int col = 0; col < 8; col++)
            {
                table->setItem(row, col, new QTableWidgetItem(query.value(col).toString()));
            }
            row++;
        }
    }
};

// ======================================================================
// MAIN FUNCTION
// ======================================================================
int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    if (!Database::initialize())
    {
        QMessageBox::critical(nullptr, "Database Error", "Failed to initialize database");
        return -1;
    }

    CarRentalApp window;
    window.show();
    return app.exec();
}

#include "main.moc"