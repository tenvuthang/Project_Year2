#include <stdio.h>
#include <math.h>
#include <string.h>

#define Bacmax 10 // Bậc đa thức tối đa

// Nguyên mẫu hàm
void nhapheso(int *bac); // Hàm nhập hệ số
void indathuc(const char *tenham, int bac, int capdaoham); // Hàm in đa thức f, f', f''
double f(double x, int bac); // Hàm f(x)
double daoham_f(double x, int bac); // Đạo hàm f'(x)
double daoham2_f(double x, int bac); // Đạo hàm f''(x)
double newton(double a, double b, double x0, double eps, int maxlap, int bac); // Thuật toán Newton
int furie(int bac, double a, double b); // Hàm kiểm tra điều kiện hội tụ Furie

// Mảng lưu hệ số toàn cục
float hs[Bacmax];

int main() {
    memset(hs, 0, sizeof(hs)); // Gán toàn bộ hệ số = 0

    int bac;
    nhapheso(&bac); // Nhập hệ số và bậc đa thức

    printf("\n============================================\n");
    indathuc("f(x)", bac, 0);   // In f(x)
    indathuc("f'(x)", bac, 1);  // In f'(x)
    indathuc("f''(x)", bac, 2); // In f''(x)
    printf("============================================\n\n");

    // Nhập khoảng phân ly nghiệm
    double a, b;
    printf(">> Nhap khoang phan ly nghiem [a,b]: ");
    scanf("%lf %lf", &a, &b);

    // Đảo lại nếu người dùng nhập ngược a > b
    if (a > b) {
        double tmp = a; a = b; b = tmp;
        printf(">> Canh bao: a > b, tu dong hoan doi lai thanh [%.3lf, %.3lf]\n", a, b);
    }

    // Nhập sai số epsilon
    double eps;
    printf(">> Nhap sai so cho phep epsilon: ");
    scanf("%lf", &eps);

    if (eps <= 0) {
        eps = 1e-6;
        printf(">> Canh bao: Sai so khong hop le, tu dong dat epsilon = 1e-6\n");
    }

    // Kiểm tra điều kiện hội tụ Furie
    int kiemTra = furie(bac, a, b);
    double x0;
    if (kiemTra == 1)
        x0 = a; // Nếu f(a)*f''(a)>0 → chọn a làm điểm bắt đầu
    else if (kiemTra == -1)
        x0 = b; // Nếu f(b)*f''(b)>0 → chọn b làm điểm bắt đầu
    else if (kiemTra == -2) {
        printf(">> Dieu kien hoi tu Furie khong duoc dam bao.\n");
        return 0;
    }

    printf(">> Bat dau Newton voi x0 = %.6lf\n", x0);

    // Gọi hàm Newton tìm nghiệm gần đúng
    newton(a, b, x0, eps, 100, bac);
    return 0;
}

// Hàm nhập hệ số đa thức
void nhapheso(int *bac) {
    printf(">> Nhap bac da thuc (<=%d): ", Bacmax);
    scanf("%d", bac);
    for (int i = *bac; i >= 0; i--) {
        printf("   He so cua x^%d: ", i);
        scanf("%f", &hs[i]);
    }
}

// Hàm in đa thức f(x), f'(x), f''(x)
void indathuc(const char *tenham, int bac, int capdaoham) {
    printf("%s = ", tenham);
    int daIn = 0;
    for (int i = bac; i >= capdaoham; i--) {
        double hsTam = hs[i];
        for (int j = 0; j < capdaoham; j++) hsTam *= (i - j); // Nhân thêm theo đạo hàm
        if (fabs(hsTam) < 1e-12) continue; // Bỏ qua hệ số ~0

        // In dấu cộng/trừ hợp lý tránh hiện dấu ở đầu
        if (daIn) printf(hsTam > 0 ? " + " : " - "); else if (hsTam < 0) printf("-");

        // Bỏ .000 nếu là số nguyên
        if (fabs(hsTam) != 1 || (i - capdaoham) == 0) {
            if (fabs(hsTam - (int)hsTam) < 1e-6)
                printf("%d", (int)fabs(hsTam));
            else
                printf("%.3f", fabs(hsTam));
        }

        // In phần x, x^2, x^3...
        if (i - capdaoham > 0) {
            printf("x");
            if (i - capdaoham > 1) printf("^%d", i - capdaoham);
        }
        daIn = 1;
    }
    if (!daIn) printf("0");
    printf("\n");
}

// Hàm tính f(x)
double f(double x, int bac) {
    double kq = 0;
    for (int i = bac; i >= 0; i--) kq += hs[i] * pow(x, i);
    return kq;
}

// Hàm tính đạo hàm cấp 1 f'(x)
double daoham_f(double x, int bac) {
    double kq = 0;
    for (int i = 1; i <= bac; i++) kq += i * hs[i] * pow(x, i - 1);
    return kq;
}

// Hàm tính đạo hàm cấp 2 f''(x)
double daoham2_f(double x, int bac) {
    double kq = 0;
    for (int i = 2; i <= bac; i++) kq += i * (i - 1) * hs[i] * pow(x, i - 2);
    return kq;
}

// Thuật toán Newton tìm nghiệm gần đúng
double newton(double a, double b, double x0, double eps, int maxlap, int bac) {
    double x = x0, saiso = 1e9; // Sai số khởi tạo lớn để bắt đầu vòng lặp
    int lap = 0;

    printf("|---------|---------------|--------|\n");
    printf("| LAN LAP | GIA TRI CUA x | SAI SO |\n");
    printf("|---------|---------------|--------|\n");

    while (saiso > eps && lap < maxlap && x >= a && x <= b) {
        double fx = f(x, bac), fdx = daoham_f(x, bac);

        if (fabs(fdx) < 1e-12) { // Nếu đạo hàm gần 0, dừng để tránh chia 0
            printf(">> Dao ham f'(x) = 0 tai x = %.6lf. Dung lai.\n", x);
            break;
        }

        double x_sau = x - fx / fdx; // Công thức Newton-Raphson
        saiso = fabs(x_sau - x); // Cập nhật sai số
        x = x_sau; // Gán lại x cho vòng lặp sau
        lap++;

        printf("|%-9d|%-15lf|%-8lf|\n", lap, x, saiso);
    }

    printf("|---------|---------------|--------|\n");

    // Thông báo kết quả
    if (saiso > eps)
        printf(">> Khong tim thay nghiem trong khoang [%.3lf, %.3lf]\n", a, b);
    else {
        printf(">> Nghiem gan dung: %f\n", x);
        printf(">> So lan lap: %d\n", lap);
        printf(">> Sai so: %lf\n", saiso);
    }

    return x;
}

// Hàm kiểm tra điều kiện hội tụ Furie (làm chặt)
int furie(int bac, double a, double b) {
    int dauDaoHam1 = daoham_f(a, bac) >= 0; // Dấu của f'(x) tại a
    for (double i = a; i <= b; i += 0.01)
        if ((daoham_f(i, bac) >= 0) != dauDaoHam1) return -2; // f' đổi dấu → không hội tụ

    int dauDaoHam2 = daoham2_f(a, bac) >= 0; // Dấu của f''(x) tại a
    for (double i = a; i <= b; i += 0.01)
        if ((daoham2_f(i, bac) >= 0) != dauDaoHam2) return -2; // f'' đổi dấu → không hội tụ

    // Chọn hướng hội tụ theo dấu f * f''
    if (f(a, bac) * daoham2_f(a, bac) > 0) return 1; // Chọn a
    if (f(b, bac) * daoham2_f(b, bac) > 0) return -1; // Chọn b

    return 0; // Không xác định được hướng
}