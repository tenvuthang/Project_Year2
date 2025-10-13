#include <iostream>
#include <vector>
#include <string>
#include <limits>

using namespace std;


struct Node {
    string name;                 // tên thư mục
    string type;                 // "FILE" or "FOLDER"
    int parentIndex;             // chỉ số cha trong mảng FileSystem
    vector<int> childrenIndices; // danh sách các con
};

vector<Node> FileSystem; // Mảng lưu trữ toàn bộ cây thư mục

// THUẬT TOÁN 1: THÊM THƯ MỤC
string Add_Folder(int parentIndex, const string &folderName) {
    // B1: Kiểm tra hợp lệ
    if (parentIndex < 0 || parentIndex >= (int)FileSystem.size() ||
        FileSystem[parentIndex].type != "FOLDER")
        return "❌ Lỗi: Thư mục cha không hợp lệ.";
    // B2: Kiểm tra trùng tên
    for (int childIndex : FileSystem[parentIndex].childrenIndices) {
        if (FileSystem[childIndex].name == folderName)
            return "❌ Lỗi: Thư mục đã tồn tại.";
    }
    // B3: Tạo node mới
    Node newNode;
    newNode.name = folderName;
    newNode.type = "FOLDER";
    newNode.parentIndex = parentIndex;

    // B4 : Thêm vào mảng chính
    FileSystem.push_back(newNode);
    int newIndex = (int)FileSystem.size() - 1;
    FileSystem[parentIndex].childrenIndices.push_back(newIndex);

    return "✅ Thêm thư mục thành công.";
}

// THUẬT TOÁN 2: THÊM TỆP TIN
string Add_File(int parentIndex, const string &fileName) {
    // B1: Kiểm tra hợp lệ
    if (parentIndex < 0 || parentIndex >= (int)FileSystem.size() ||
        FileSystem[parentIndex].type != "FOLDER")
        return "❌ Lỗi: Thư mục cha không hợp lệ.";

    // B2: Kiểm tra trùng tên
    for (int childIndex : FileSystem[parentIndex].childrenIndices) {
        if (FileSystem[childIndex].name == fileName)
            return "❌ Lỗi: File đã tồn tại.";
    }

    // B3: Tạo node mới
    Node newNode;
    newNode.name = fileName;
    newNode.type = "FILE";
    newNode.parentIndex = parentIndex;

    // B4 : Thêm vào mảng chính
    FileSystem.push_back(newNode);
    int newIndex = (int)FileSystem.size() - 1;
    FileSystem[parentIndex].childrenIndices.push_back(newIndex);

    return "✅ Thêm file thành công.";
}

// THUẬT TOÁN 3: Duyệt & in cây
// B1: In cây thư mục
void Display_Tree(int nodeIndex, const string &prefix = "", bool isLast = true) {
    if (nodeIndex < 0 || nodeIndex >= (int)FileSystem.size())
        return;

    const Node &node = FileSystem[nodeIndex];

    cout << prefix;
    if (nodeIndex != 0) {
        cout << (isLast ? "└── " : "├── ");
    }

    // B2: In thông tin nút hiện tại
    if (node.type == "FILE")
        cout << "[" << nodeIndex << "] 📄 " << node.name << endl;
    else
        cout << "[" << nodeIndex << "] 📁 " << node.name << "/" << endl;


    // B3: Duyệt và in các con
    for (int i = 0; i < (int)node.childrenIndices.size(); i++) {
        // Xác định đây có phải là con cuối cùng của node hiện tại hay không
        bool lastChild = (i == (int)node.childrenIndices.size() - 1);

        // Cập nhật prefix cho con
        string nextPrefix = prefix + (nodeIndex == 0 ? "" : (isLast ? "    " : "│   "));

        // Gọi đệ quy để in toàn bộ cây con
        Display_Tree(node.childrenIndices[i], nextPrefix, lastChild);
    }
}
// Hàm tìm kiếm theo tên (bổ sung)
void Search_NodeByName(const string &keyword) {
    bool found = false;
    cout << "Kết quả tìm kiếm cho \"" << keyword << "\":\n";
    for (int i = 0; i < (int)FileSystem.size(); i++) {
        if (FileSystem[i].name.find(keyword) != string::npos) {
            cout << "[" << i << "] "
                 << (FileSystem[i].type == "FILE" ? "📄 " : "📁 ")
                 << FileSystem[i].name;

            int p = FileSystem[i].parentIndex;
            if (p >= 0)
                cout << " (trong: " << FileSystem[p].name << ")\n";
            else
                cout << " (thư mục gốc)\n";

            found = true;
        }
    }
    if (!found) cout << "❌ Không tìm thấy kết quả nào.\n";
}

// Tạo một số dữ liệu mẫu để tiện demo
void Mau() {
    cout << Add_Folder(0, "Documents") << endl;
    cout << Add_Folder(0, "Pictures") << endl;
    cout << Add_File(1, "Report.docx") << endl;
    cout << Add_File(2, "Photo.png") << endl;
    cout << Add_Folder(1, "Homework") << endl;
    cout << Add_File(4, "BaiTap1.cpp") << endl;
}

// Menu thao tác (bổ sung)
int main() {
    // Tạo root
    Node root;
    root.name = "Root";
    root.type = "FOLDER";
    root.parentIndex = -1;
    FileSystem.push_back(root);

    // Tạo dữ liệu mẫu
    Mau();

    cout << "Trình chỉnh sửa cây thư mục đơn giản. Nhập lệnh để chỉnh sửa hệ thống tệp ảo.\n";
    cout << "Lệnh:\n";
    cout << "  1 - Thêm thư mục\n";
    cout << "  2 - Thêm tệp\n";
    cout << "  3 - Hiển thị cây thư mục\n";
    cout << "  4 - Trợ giúp\n";
    cout << "  5 - Tìm kiếm tên\n";
    cout << "  0 - Thoát\n";

    while (true) {
        cout << "\n> ";
        int cmd;
        if (!(cin >> cmd)) {
            // EOF hoặc đầu vào lỗi -> thoát vòng lặp
            break;
        }

        if (cmd == 0) {
            break;
        }

        switch (cmd) {
            case 1: {
                int parent;
                cout << "Chỉ số thư mục cha: ";
                if (!(cin >> parent)) {
                    cout << "Chỉ số thư mục cha không hợp lệ.\n";
                    if (cin.eof()) goto finish;
                    cin.clear();
                    string dummy;
                    getline(cin, dummy);
                    break;
                }
                // loại bỏ khoảng trắng còn sót lại trước getline
                cin.ignore(numeric_limits<streamsize>::max(), '\n');
                cout << "Tên thư mục: ";
                string name;
                if (!std::getline(cin, name)) {
                    if (cin.eof()) goto finish;
                    break;
                }
                if (name.empty()) {
                    cout << "Tên thư mục không được rỗng.\n";
                    break;
                }
                cout << Add_Folder(parent, name) << endl;
                break;
            }
            case 2: {
                int parent;
                cout << "Chỉ số thư mục cha: ";
                if (!(cin >> parent)) {
                    cout << "Chỉ số thư mục cha không hợp lệ.\n";
                    if (cin.eof()) goto finish;
                    cin.clear();
                    string dummy;
                    getline(cin, dummy);
                    break;
                }
                cin.ignore(numeric_limits<streamsize>::max(), '\n');
                cout << "Tên tệp: ";
                string name;
                if (!std::getline(cin, name)) {
                    if (cin.eof()) goto finish;
                    break;
                }
                if (name.empty()) {
                    cout << "Tên tệp không được rỗng.\n";
                    break;
                }
                cout << Add_File(parent, name) << endl;
                break;
            }
            case 3:
                cout << "\n===== CẤU TRÚC CÂY THƯ MỤC =====\n";
                Display_Tree(0);
                break;
            case 4:
                cout << "Lệnh:\n";
                cout << "  1 - Thêm thư mục\n";
                cout << "  2 - Thêm tệp\n";
                cout << "  3 - Hiển thị cây thư mục\n";
                cout << "  4 - Trợ giúp\n";
                cout << "  5 - Tìm kiếm tên\n";
                cout << "  0 - Thoát\n";
                break;
            case 5: {
                cin.ignore(numeric_limits<streamsize>::max(), '\n');
                cout << "Nhập tên cần tìm: ";
                string keyword;
                getline(cin, keyword);
                if (!keyword.empty())
                    Search_NodeByName(keyword);
                break;
            }
            default:
                cout << "Lệnh không hợp lệ. Nhập 4 để xem trợ giúp.\n";
                break;
        }
    }
finish:
    cout << "Thoát. Cây thư mục cuối cùng:\n";
    Display_Tree(0);
    return 0;
}