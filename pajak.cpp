#include <iostream>
#include <fstream>
#include <string>
#include <algorithm> // Untuk std::sort
#include <iomanip>   // Untuk std::setw, std::fixed, std::setprecision
#include <thread>
#include <chrono>
using namespace std;

#define MAX_USERS 100 // Batas maksimum jumlah pengguna

// ===== STRUCT =====
struct User {
    string username;
    string password;
    string nik;
    string name;
    double income = 0.0;
    double propertyValue = 0.0;
    double vehicleValue = 0.0;
    int dependents = 0;
    bool payment = false; // Status pembayaran (false = belum, true = sudah)
    bool isAdmin = false;
};


// ===== GLOBAL VAR =====
bool isLoggedIn = false;
string currentUser;
bool isAdmin = false;
User loggedInUser;
User users[MAX_USERS]; // Array C-style untuk menyimpan data user
int userCount = 0;     // Jumlah user saat ini
string filename = "user.txt"; // Nama file (gunakan nama berbeda)

// ===== FUNCTION DECLARATION =====
int integerDetection();
void registerUser();
void loginUser();
void logoutUser();
void detectRole();
void showUserMenu();
void showAdminMenu();
void viewProfile();
void calculateTax();
double calculateTaxPPh21(double monthlyIncome, int dependents);
double calculatePropertyTax(double propertyValue);
double calculateVehicleTax(double vehicleValue);
void updatePaymentStatus();
void viewTaxReport();
void viewAllUsers();
void searchUser();
void editUserData();
void sortUsersByTax();
void updateUserPaymentManually();
void readAllUsers(); // Membaca semua user dari file ke array
void writeAllUsers(); // Menulis semua user dari array ke file
void parseLine(const string& line, User& u); // Parsing manual
double calculateTotalTax(const User& user);
bool compareUsersByTax(const User& a, const User& b); // Komparator untuk sort
bool checkUsernameAvailability(const string& username); // NEW: Deklarasi fungsi baru
int searchUserByNikRecursive(const string& nik, int index); // NEW: Deklarasi fungsi baru
bool checkUsernameAvailability(const string& username);
bool checkNikAvailability(const string& nik); // NEW: Deklarasi fungsi baru

// ===== MAIN =====
int main() {
    readAllUsers(); // Muat data pengguna saat program dimulai
    int choice;

    while (true) {
        cout << "\n=== SISTEM PAJAK (C++ IO/String) ===\n";
        cout << "1. Login\n";
        cout << "2. Register\n";
        cout << "3. Exit\n";
        cout << "Pilih: ";
        choice = integerDetection();

        if (cin.fail()) {
            cout << "Input tidak valid. Harap masukkan angka.\n";
            cin.clear();
            cin.ignore(10000, '\n');
            continue;
        }
        cin.ignore(10000, '\n'); // Abaikan newline setelah angka

        switch (choice) {
            case 1:
                loginUser();
                if (isLoggedIn) {
                    detectRole();
                    if (isAdmin)
                        showAdminMenu();
                    else
                        showUserMenu();
                }
                break;
            case 2: registerUser(); break;
            case 3: cout << "Keluar...\n"; return 0;
            default: cout << "Pilihan tidak valid.\n";
        }
    }
}


// ====== Loading ======
void showLoading(string message, int dotCount = 3, int delayMs = 500) {
    cout << message;
    for (int i = 0; i < dotCount; ++i) {
        cout << ".";
        cout.flush(); // Pastikan output langsung tampil
        std::this_thread::sleep_for(std::chrono::milliseconds(delayMs)); // Tambahkan std::
    }
    cout << endl;
}

// ===== Penghasilan kurang dari PTKP =====
bool isExemptedFromPPh21(const User& user) {
    return user.income < 4500000; // PTKP misalnya 4.5 juta
}

bool hasPropertyOrVehicle(const User& user) {
    return user.propertyValue > 0 || user.vehicleValue > 0;
}

// NEW: Function to check if username already exists
bool checkUsernameAvailability(const string& username) {
    for (int i = 0; i < userCount; ++i) {
        if (users[i].username == username) {
            return false; // Username already exists
        }
    }
    return true; // Username is available
}

// ===== REGISTER =====
void registerUser() {
    if (userCount >= MAX_USERS) {
        cout << "Maaf, batas maksimum pengguna telah tercapai.\n";
        return;
    }

    User u;
    string confirm;

    cout << "\n--- REGISTER ---\n";
    while (true) {
        cout << "Username        : ";
        cin >> u.username;
        if (!checkUsernameAvailability(u.username)) {
            cout << "Username sudah ada. Harap pilih username lain.\n";
            cin.ignore(numeric_limits<streamsize>::max(), '\n'); // Gunakan ini untuk membersihkan buffer
        } else {
            break; // Username tersedia, lanjutkan
        }
    }

    cout << "Password        : "; cin >> u.password;
    cout << "Konfirmasi Pass : "; cin >> confirm;

    if (u.password != confirm) {
        cout << "Password tidak cocok!\n";
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        return;
    }

    cout << "Nama Lengkap    : "; cin.ignore(); getline(cin, u.name);

    // NEW: Loop untuk memeriksa NIK
    while (true) {
        cout << "NIK             : ";
        cin >> u.nik;
        if (!checkNikAvailability(u.nik)) {
            cout << "NIK sudah terdaftar. Harap masukkan NIK lain.\n";
            cin.ignore(numeric_limits<streamsize>::max(), '\n'); // Membersihkan buffer
        } else {
            break; // NIK tersedia, lanjutkan
        }
    }

    cout << "Penghasilan/bln : "; u.income=integerDetection();
    cout << "Jumlah tanggungan: "; u.dependents=integerDetection();

    char jawab;
    cout << "Punya properti? (y/n): "; cin >> jawab;
    if (jawab == 'y' || jawab == 'Y') {
        cout << "  Nilai properti : "; u.propertyValue=integerDetection();
    }

    cout << "Punya kendaraan? (y/n): "; cin >> jawab;
    if (jawab == 'y' || jawab == 'Y') {
        cout << "  Nilai kendaraan: "; u.vehicleValue=integerDetection();
    }

    u.isAdmin = false;
    u.payment = false;

    users[userCount] = u; // Tambahkan ke array
    userCount++;
    writeAllUsers(); // Simpan ke file
    cout << "Registrasi berhasil!\n";
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
}

// ===== INT DETECTION =====
int integerDetection(){
    int input;
    while (!(cin >> input))
    {
        cin.clear();
        cin.ignore(1000, '\n');
        cout << "Tolong masukkan input berupa integer: ";
    }
    return input;
}

// ===== LOGIN =====
void loginUser() {
    string uname, pass;
    cout << "\n--- LOGIN ---\n";
    cout << "Username: "; cin >> uname;
    cout << "Password: "; cin >> pass;
    cin.ignore(10000, '\n');

    if (uname == "admin" && pass == "admin123") {
        isLoggedIn = true;
        isAdmin = true;
        currentUser = uname;
        loggedInUser.username = "admin";
        loggedInUser.name = "Administrator";
        loggedInUser.isAdmin = true;
        cout << "✅ Login berhasil! Selamat datang, Admin.\n";
        return;
    }

    // readAllUsers(); // Sudah di main
    bool found = false;
    for (int i = 0; i < userCount; i++) {
        if (uname == users[i].username && pass == users[i].password) {
            isLoggedIn = true;
            isAdmin = users[i].isAdmin;
            currentUser = users[i].username;
            loggedInUser = users[i]; // Salin data
            cout << "Login berhasil. Selamat datang, " << users[i].name << "!\n";
            found = true;
            break;
        }
    }

    if (!found) {
        cout << "Login gagal. Username/password salah.\n";
    }
}

// ===== LOGOUT =====
void logoutUser() {
    cout << "Logout berhasil!\n";
    isLoggedIn = false;
    isAdmin = false;
    currentUser = "";
    loggedInUser = User(); // Reset
}

// ===== DETECT ROLE =====
void detectRole() {
    cout << "Anda login sebagai: " << (isAdmin ? "ADMIN" : "USER") << "\n";
}

// ===== MENU USER =====
// ===== MENU USER (Rekursif) =====
void showUserMenu() {
    if (!isLoggedIn || isAdmin) { // Kondisi basis: jika tidak login atau adalah admin, hentikan rekursi
        return;
    }
    int ch;
    cout << "\n--- MENU USER ---\n";
    cout << "1. Lihat Profil\n";
    cout << "2. Hitung & Lihat Pajak\n";
    cout << "3. Update Status Bayar Pajak\n";
    cout << "4. Lihat Laporan Pajak\n";
    cout << "5. Logout\n";
    cout << "Pilih: ";
    ch = integerDetection();

    if (cin.fail()) {
        cout << "Input salah.\n";
        cin.clear();
        cin.ignore(10000, '\n');
        showUserMenu(); // Panggil diri sendiri lagi jika input salah
        return;
    }
    cin.ignore(10000, '\n');

    switch (ch) {
        case 1:
            viewProfile();
            showUserMenu(); // Panggil diri sendiri untuk kembali ke menu
            break;
        case 2:
            calculateTax();
            showUserMenu(); // Panggil diri sendiri untuk kembali ke menu
            break;
        case 3:
            updatePaymentStatus();
            showUserMenu(); // Panggil diri sendiri untuk kembali ke menu
            break;
        case 4:
            viewTaxReport();
            showUserMenu(); // Panggil diri sendiri untuk kembali ke menu
            break;
        case 5:
            logoutUser();
            // Setelah logout, rekursi akan berhenti karena kondisi basis !isLoggedIn
            return;
        default:
            cout << "Pilihan salah.\n";
            showUserMenu(); // Panggil diri sendiri untuk kembali ke menu
            break;
    }
}

// ===== MENU ADMIN =====
void showAdminMenu() {
    int ch;
    while (isLoggedIn && isAdmin) {
        cout << "\n--- MENU ADMIN ---\n";
        cout << "1. Lihat Semua User\n";
        cout << "2. Cari User\n";
        cout << "3. Edit Data User\n";
        cout << "4. Sorting User Berdasarkan Pajak\n";
        cout << "5. Update Status Pembayaran User\n";
        cout << "6. Logout\n";
        cout << "Pilih: ";
        cin >> ch;
        if (cin.fail()) { cout << "Input salah.\n"; cin.clear(); cin.ignore(10000, '\n'); continue; }
        cin.ignore(10000, '\n');

        switch (ch) {
            case 1: viewAllUsers(); break;
            case 2: searchUser(); break;
            case 3: editUserData(); break;
            case 4: sortUsersByTax(); break;
            case 5: updateUserPaymentManually(); break;
            case 6: logoutUser(); return;
            default: cout << "Pilihan salah.\n";
        }
    }
}

// ===== UTILITAS USER =====
// NEW: Function to check if NIK already exists
bool checkNikAvailability(const string& nik) {
    for (int i = 0; i < userCount; ++i) {
        if (users[i].nik == nik) {
            return false; // NIK sudah ada
        }
    }
    return true; // NIK tersedia
}
void viewProfile() {
    cout << "\n--- PROFIL ANDA ---\n";
    cout << "Username      : " << loggedInUser.username << "\n";
    cout << "Nama          : " << loggedInUser.name << "\n";
    cout << "NIK           : " << loggedInUser.nik << "\n";
    cout << "Penghasilan   : Rp " << fixed << setprecision(2) << loggedInUser.income << "\n";
    cout << "Properti      : Rp " << fixed << setprecision(2) << loggedInUser.propertyValue << "\n";
    cout << "Kendaraan     : Rp " << fixed << setprecision(2) << loggedInUser.vehicleValue << "\n";
    cout << "Tanggungan    : " << loggedInUser.dependents << "\n";
    cout << "Status Pajak  : " << (loggedInUser.payment ? "SUDAH BAYAR" : "BELUM BAYAR") << "\n";

    bool exemptPPh21 = isExemptedFromPPh21(loggedInUser);
    bool hasAssets = hasPropertyOrVehicle(loggedInUser);

    if (exemptPPh21 && !hasAssets) {
        cout << "\n⚠️ Anda tidak diwajibkan membayar pajak.\n";
        return;
    }
}

void calculateTax() {
    bool exemptPPh21 = isExemptedFromPPh21(loggedInUser);
    bool hasAssets = hasPropertyOrVehicle(loggedInUser);

    if (exemptPPh21 && !hasAssets) {
        cout << "\n⚠️ Anda tidak diwajibkan membayar pajak.\n";
        return;
    }

    double pph21 = exemptPPh21 ? 0 : calculateTaxPPh21(loggedInUser.income, loggedInUser.dependents);
    double propertyTax = calculatePropertyTax(loggedInUser.propertyValue);
    double vehicleTax = calculateVehicleTax(loggedInUser.vehicleValue);
    double totalTax = pph21 + propertyTax + vehicleTax;

    cout << "\n--- PERHITUNGAN PAJAK ANDA ---" << endl;
    cout << "Pajak Penghasilan (PPh 21) / Tahun : Rp " << fixed << setprecision(2) << pph21 << endl;
    cout << "Pajak Properti / Tahun           : Rp " << fixed << setprecision(2) << propertyTax << endl;
    cout << "Pajak Kendaraan / Tahun          : Rp " << fixed << setprecision(2) << vehicleTax << endl;
    cout << "------------------------------------------" << endl;
    cout << "Total Pajak Tahunan              : Rp " << fixed << setprecision(2) << totalTax << endl;
}

void updatePaymentStatus() {
    bool exemptPPh21 = isExemptedFromPPh21(loggedInUser);
    bool hasAssets = hasPropertyOrVehicle(loggedInUser);

    if (exemptPPh21 && !hasAssets) {
        cout << "\n⚠️ Anda tidak diwajibkan membayar pajak, sehingga tidak perlu update status bayar pajak.\n";
        return;
    }

    if (loggedInUser.payment) {
        cout << "Anda sudah membayar pajak tahun ini. ✅\n";
        return;
    }

    double total = calculateTotalTax(loggedInUser);
    cout << "\nJumlah pajak yang harus dibayar: Rp " << fixed << setprecision(2) << total << endl;
    cout << "Lanjutkan ke pembayaran? (y/n): ";
    char confirm;
    cin >> confirm;
    if (confirm != 'y' && confirm != 'Y') {
        cout << "Pembayaran dibatalkan.\n";
        return;
    }

    // Tampilkan QR Code
    cout << "Silakan scan QR Code berikut untuk melakukan pembayaran:\n";
#ifdef _WIN32
    system("start qr_pembayaran.png"); // Windows
#elif __APPLE__
    system("open qr_pembayaran.png");  // macOS
#else
    system("xdg-open qr_pembayaran.png"); // Linux
#endif

    // Simulasi pembayaran setelah QR dibuka
    cout << "\nTekan Enter setelah Anda menyelesaikan pembayaran...";
    cin.ignore();
    cin.get(); // tunggu input enter

    showLoading("Memproses pembayaran", 5, 800);
    cout << "Pembayaran berhasil! ✅\n";

    // Update status
    for (int i = 0; i < userCount; i++) {
        if (users[i].username == loggedInUser.username) {
            users[i].payment = true;
            loggedInUser.payment = true;
            break;
        }
    }
    writeAllUsers();
}

void viewTaxReport() {
    cout << "\n--- LAPORAN PAJAK TAHUNAN ---" << endl;
    cout << "Username      : " << loggedInUser.username << "\n";
    cout << "Nama          : " << loggedInUser.name << "\n";
    cout << "NIK           : " << loggedInUser.nik << "\n";
    cout << "------------------------------------------" << endl;

    bool exemptPPh21 = isExemptedFromPPh21(loggedInUser);
    bool hasAssets = hasPropertyOrVehicle(loggedInUser);

    if (exemptPPh21 && !hasAssets) {
        cout << "\n⚠️ Anda tidak diwajibkan membayar pajak.\n";
        return;
    }

    calculateTax();
    cout << "------------------------------------------" << endl;
    cout << "Status Pembayaran : " << (loggedInUser.payment ? "✅ SUDAH BAYAR" : "❌ BELUM BAYAR") << endl;
}

// ===== UTILITAS ADMIN =====
int searchUserByNikRecursive(const string& nik, int index) {
    if (index >= userCount)
        return -1; // Basis: tidak ditemukan
    if (users[index].nik == nik)
        return index; // Basis: ditemukan
    return searchUserByNikRecursive(nik, index + 1); // Rekursi
}

// Fungsi pengecekan wajib pajak (income atau properti/kendaraan)
bool isRequiredToPayTax(const User& user) {
    if (user.income >= 4500000) return true;
    if (user.propertyValue > 0) return true;
    if (user.vehicleValue > 0) return true;
    return false;
}

void viewAllUsers() {
    cout << "\n--- DAFTAR SEMUA USER ---\n";
    if (userCount == 0) {
        cout << "Tidak ada data user terdaftar.\n";
        return;
    }

    cout << left << setw(15) << "Username"
         << setw(25) << "Nama Lengkap"
         << setw(20) << "NIK"
         << setw(15) << "Income"
         << setw(15) << "Status Bayar"
         << setw(15) << "Wajib Pajak" << endl;
    cout << string(100, '-') << endl;

    for (int i = 0; i < userCount; i++) {
        cout << left << setw(15) << users[i].username
             << setw(25) << users[i].name
             << setw(20) << users[i].nik
             << setw(15) << fixed << setprecision(0) << users[i].income
             << setw(15) << (users[i].payment ? "Sudah" : "Belum")
             << setw(15) << (isRequiredToPayTax(users[i]) ? "Wajib" : "Tidak") << endl;
    }
    cout << string(100, '-') << endl;
}

void searchUser() {
      string nik;
    cout << "Masukkan NIK user yang dicari: "; // Berubah dari 'username' menjadi 'NIK'
    cin >> nik;
    cin.ignore(10000, '\n');

    int index = searchUserByNikRecursive(nik, 0); // Memanggil fungsi pencarian berdasarkan NIK

    if (index == -1) {
        cout << "User dengan NIK tersebut tidak ditemukan.\n"; // Pesan disesuaikan
    } else {
        cout << "\n--- User Ditemukan ---" << endl;
        cout << "Username      : " << users[index].username << endl;
        cout << "Nama          : " << users[index].name << endl;
        cout << "NIK           : " << users[index].nik << endl;
        cout << "Penghasilan   : Rp " << fixed << setprecision(2) << users[index].income << endl;
        cout << "Properti      : Rp " << fixed << setprecision(2) << users[index].propertyValue << endl;
        cout << "Kendaraan     : Rp " << fixed << setprecision(2) << users[index].vehicleValue << endl;
        cout << "Tanggungan    : " << users[index].dependents << endl;
        cout << "Status Bayar  : " << (users[index].payment ? "SUDAH" : "BELUM") << endl;
    }
}

void editUserData() {
    string nik;
    cout << "Masukkan NIK user yang akan diedit: "; // Berubah dari 'username' menjadi 'NIK'
    cin >> nik;
    cin.ignore(10000, '\n');

    int userIndex = -1;
    // Menggunakan fungsi searchUserByNikRecursive untuk mencari user berdasarkan NIK
    userIndex = searchUserByNikRecursive(nik, 0);

    if (userIndex == -1) {
        cout << "User dengan NIK tersebut tidak ditemukan.\n"; // Pesan disesuaikan
        return;
    }

    cout << "User ditemukan. Pilih data yang ingin diedit:" << endl;
    cout << "1. Nama Lengkap\n2. NIK\n3. Penghasilan\n4. Nilai Properti\n";
    cout << "5. Nilai Kendaraan\n6. Jumlah Tanggungan\n7. Password\n0. Batal\n";
    cout << "Pilih: ";
    int choice;
    cin >> choice;
    if (cin.fail()) { cout << "Input salah.\n"; cin.clear(); cin.ignore(10000, '\n'); return; }
    cin.ignore(10000, '\n');

    switch (choice) {
        case 1: cout << "Nama baru: "; getline(cin, users[userIndex].name); break;
        case 2: cout << "NIK baru: "; cin >> users[userIndex].nik; cin.ignore(); break;
        case 3: cout << "Penghasilan baru: "; cin >> users[userIndex].income; cin.ignore(); break;
        case 4: cout << "Nilai properti baru: "; cin >> users[userIndex].propertyValue; cin.ignore(); break;
        case 5: cout << "Nilai kendaraan baru: "; cin >> users[userIndex].vehicleValue; cin.ignore(); break;
        case 6: cout << "Jumlah tanggungan baru: "; cin >> users[userIndex].dependents; cin.ignore(); break;
        case 7: cout << "Password baru: "; cin >> users[userIndex].password; cin.ignore(); break;
        case 0: cout << "Edit dibatalkan.\n"; return;
        default: cout << "Pilihan tidak valid.\n"; return;
    }

    writeAllUsers();
    cout << "Data user berhasil diperbarui.\n";
}

void sortUsersByTax() {
     if (userCount == 0) {
        cout << "Tidak ada data user untuk diurutkan.\n";
        return;
    }

    // Bubble Sort
    for (int i = 0; i < userCount - 1; i++) {
        for (int j = 0; j < userCount - i - 1; j++) {
            if (calculateTotalTax(users[j]) < calculateTotalTax(users[j + 1])) {
                // Tukar posisi users[j] dan users[j + 1]
                User temp = users[j];
                users[j] = users[j + 1];
                users[j + 1] = temp;
            }
        }
    }

    cout << "\n--- USER BERDASARKAN PAJAK (TERBESAR KE TERKECIL) ---\n";
    cout << left << setw(15) << "Username"
         << setw(25) << "Nama Lengkap"
         << setw(20) << "Total Pajak (Rp)" << endl;
    cout << string(60, '-') << endl;

    for (int i=0; i < userCount; i++) {
         cout << left << setw(15) << users[i].username
             << setw(25) << users[i].name
             << setw(20) << fixed << setprecision(2) << calculateTotalTax(users[i]) << endl;
    }
     cout << string(60, '-') << endl;
}


void updateUserPaymentManually() {
    string uname;
    cout << "Masukkan username user yang statusnya akan diubah: ";
    cin >> uname;
    cin.ignore(10000, '\n');

    int userIndex = -1;
    for (int i = 0; i < userCount; i++) {
        if (users[i].username == uname) {
            userIndex = i;
            break;
        }
    }

    if (userIndex == -1) {
        cout << "User tidak ditemukan.\n";
        return;
    }

    cout << "User: " << users[userIndex].name << " (" << users[userIndex].username << ")" << endl;
    cout << "Status saat ini: " << (users[userIndex].payment ? "SUDAH BAYAR" : "BELUM BAYAR") << endl;
    cout << "Ubah status? (y = Sudah Bayar, n = Belum Bayar, c = Batal): ";
    char choice;
    cin >> choice;
    cin.ignore(10000, '\n');

    if (choice == 'y' || choice == 'Y') {
        users[userIndex].payment = true;
        cout << "Status diubah menjadi SUDAH BAYAR.\n";
        writeAllUsers();
    } else if (choice == 'n' || choice == 'N') {
        users[userIndex].payment = false;
        cout << "Status diubah menjadi BELUM BAYAR.\n";
        writeAllUsers();
    } else {
        cout << "Perubahan dibatalkan.\n";
    }
}

// ===== FILE HANDLING (C++ fstream, manual parsing) =====
void parseLine(const string& line, User& u) {
    string buffer = line;
    size_t pos = 0;
    string token;
    string delimiter = "|";
    string fields[10]; // Array untuk menampung field
    int count = 0;

    // Pecah string berdasarkan delimiter '|'
    while ((pos = buffer.find(delimiter)) != string::npos && count < 9) {
        token = buffer.substr(0, pos);
        fields[count++] = token;
        buffer.erase(0, pos + delimiter.length());
    }
    fields[count++] = buffer; // Field terakhir

    // Assign ke struct User dengan penanganan error/nilai default
    u.username = (count > 0) ? fields[0] : "";
    u.password = (count > 1) ? fields[1] : "";
    u.nik = (count > 2) ? fields[2] : "";
    u.name = (count > 3) ? fields[3] : "";
    try { u.income = (count > 4 && !fields[4].empty()) ? stod(fields[4]) : 0.0; } catch (...) { u.income = 0.0; }
    try { u.dependents = (count > 5 && !fields[5].empty()) ? stoi(fields[5]) : 0; } catch (...) { u.dependents = 0; }
    try { u.propertyValue = (count > 6 && !fields[6].empty()) ? stod(fields[6]) : 0.0; } catch (...) { u.propertyValue = 0.0; }
    try { u.vehicleValue = (count > 7 && !fields[7].empty()) ? stod(fields[7]) : 0.0; } catch (...) { u.vehicleValue = 0.0; }
    try { u.isAdmin = (count > 8 && !fields[8].empty()) ? (stoi(fields[8]) != 0) : false; } catch (...) { u.isAdmin = false; }
    try { u.payment = (count > 9 && !fields[9].empty()) ? (stoi(fields[9]) != 0) : false; } catch (...) { u.payment = false; }
}

void readAllUsers() {
    ifstream file(filename);
    string line;
    userCount = 0;

    if (!file.is_open()) {
        // Jika file tidak ada, tidak apa-apa
        return;
    }

    while (getline(file, line) && userCount < MAX_USERS) {
        if (!line.empty()) {
            parseLine(line, users[userCount]);
            userCount++;
        }
    }
    file.close();
}

void writeAllUsers() {
    ofstream file(filename, ios::trunc); // Overwrite
    if (!file.is_open()) {
        cerr << "Error: Tidak bisa membuka file " << filename << " untuk ditulis.\n";
        return;
    }

    for (int i = 0; i < userCount; i++) {
        file << users[i].username << "|"
             << users[i].password << "|"
             << users[i].nik << "|"
             << users[i].name << "|"
             << users[i].income << "|"
             << users[i].dependents << "|"
             << users[i].propertyValue << "|"
             << users[i].vehicleValue << "|"
             << users[i].isAdmin << "|"
             << users[i].payment << "\n";
    }
    file.close();
}

// ===== TAX CALCULATION (Sama seperti sebelumnya) =====
double calculateTaxPPh21(double monthlyIncome, int dependents) {
    double annualIncome = monthlyIncome * 12;
    int maxDependents = min(dependents, 3);
    double ptkp = 54000000 + (maxDependents * 4500000);
    double pkp = annualIncome - ptkp;
    if (pkp <= 0) return 0;
    double tax = 0;
    if (pkp <= 60000000) tax = pkp * 0.05;
    else if (pkp <= 250000000) tax = (60000000 * 0.05) + ((pkp - 60000000) * 0.15);
    else if (pkp <= 500000000) tax = (60000000 * 0.05) + (190000000 * 0.15) + ((pkp - 250000000) * 0.25);
    else if (pkp <= 5000000000) tax = (60000000 * 0.05) + (190000000 * 0.15) + (250000000 * 0.25) + ((pkp - 500000000) * 0.30);
    else tax = (60000000 * 0.05) + (190000000 * 0.15) + (250000000 * 0.25) + (4500000000 * 0.30) + ((pkp - 5000000000) * 0.35);
    return tax;
}

double calculatePropertyTax(double propertyValue) {
    return 0.001 * propertyValue;
}

double calculateVehicleTax(double vehicleValue) {
    return 0.02 * vehicleValue;
}

double calculateTotalTax(const User& user) {
    double pph21 = isExemptedFromPPh21(user) ? 0 : calculateTaxPPh21(user.income, user.dependents);
    double propertyTax = calculatePropertyTax(user.propertyValue);
    double vehicleTax = calculateVehicleTax(user.vehicleValue);
    return pph21 + propertyTax + vehicleTax;
}

// Fungsi komparator untuk std::sort
bool compareUsersByTax(const User& a, const User& b) {
    return calculateTotalTax(a) > calculateTotalTax(b);
}
