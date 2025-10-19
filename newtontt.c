#include <stdio.h>
#include <math.h>
#include <string.h>

#define Bacmax 10 // Bậc đa thức tối đa

// Nguyên mẫu hàm
void nhapheso(int *bac); // Hàm nhập hệ số
void indathuc(const char *tenham, int bac, int capdaoham); // Hàm in đa thức 
double f(double x, int bac); // Hàm f(x)
double daoham_f(double x, int bac); // Đạo hàm f'(x)
double daoham2_f(double x, int bac); // Đạo hàm f''(x)
double newton(double a, double b, double x0, double eps, int maxlap, int bac); // Hàm Newton
int furie(int bac, double a, double b); // Hàm kiểm tra điều kiện hội tụ

// Biến toàn cục
float hs[Bacmax]; // Mảng hệ số đa thức

// Hàm main
int main() {
    memset(hs, 0, sizeof(hs)); // Gán toàn bộ hệ số = 0

    int bac;
    nhapheso(&bac); // Nhập hệ số đa thức
    printf("\n============================================\n");
    indathuc("f(x)", bac, 0);   // In f(x)
    indathuc("f'(x)", bac, 1);  // In f'(x)
    indathuc("f''(x)", bac, 2); // In f''(x)
    printf("============================================\n\n");
    fflush(stdout); // Xóa bộ nhớ đệm, đảm bảo không in lặp


    double a, b;
    printf("Nhap khoang phan li nghiem [a,b]: ");
    scanf("%lf %lf", &a, &b);

    double eps;
    printf("Nhap sai so cho phep epsilon: ");
    scanf("%lf", &eps);

    // Kiểm tra điều kiện hội tụ Furie
    int check = furie(bac, a, b);
    if (check == 0) {
        printf(">> Canh bao: Dieu kien hoi tu khong chac chan, van thu chay Newton...\n");
    }

    // Chọn điểm bắt đầu x0
    double x0 = (check == 1) ? a : b;
    int maxlap = 100; // Giới hạn số vòng lặp

    // Gọi thuật toán Newton
    newton(a, b, x0, eps, maxlap, bac);

    return 0;
}

// Hàm nhập hệ số
void nhapheso(int *bac) {
    printf("Nhap bac da thuc (<=%d): ", Bacmax);
    scanf("%d", bac);
    for (int i = *bac; i >= 0; i--) {
        printf("He so cua x^%d: ", i);
        scanf("%f", &hs[i]);
    }
}

// Hàm in đa thức để kiểm tra lại
void indathuc(const char *tenham, int bac, int capdaoham) {
    // tenham: f(x), f'(x), f''(x)
    // bac: bậc đa thức gốc
    // capdaoham: 0 -> f(x), 1 -> f'(x), 2 -> f''(x)

    printf("%s = ", tenham);

    int printed = 0;
    for (int i = bac; i >= capdaoham; i--) {
        // Tính hệ số đạo hàm tương ứng
        double hsTam = hs[i];
        for (int j = 0; j < capdaoham; j++)
            hsTam *= (i - j);
        if (fabs(hsTam) < 1e-12) continue; // bỏ qua hệ số 0

        // In dấu cộng/trừ hợp lý tránh hiện dấu ở ngay đầu
        if (printed) {
            if (hsTam > 0) printf(" + ");
            else printf(" - ");
        } else if (hsTam < 0) printf("-");
        
        // In hệ số (bỏ .000 nếu là số nguyên)
        if (fabs(hsTam) != 1 || (i - capdaoham) == 0) {
            if (fabs(hsTam - (int)hsTam) < 1e-6)
                printf("%d", (int)fabs(hsTam));
            else
                printf("%.3f", fabs(hsTam));
        }

        // In phần x, x^2, x^3...
        if (i - capdaoham > 0) {
            printf("x");
            if (i - capdaoham > 1)
                printf("^%d", i - capdaoham);
        }

        printed = 1;
    }

    if (!printed) printf("0"); // nếu tất cả hệ số = 0
    printf("\n");
}

// Hàm f(x)
double f(double x, int bac) {
    double kq = 0;
    for (int i = bac; i >= 0; i--) {
        kq += hs[i] * pow(x, i);
    }
    return kq;
}

// Hàm f'(x)
double daoham_f(double x, int bac) {
    double kq = 0;
    for (int i = 1; i <= bac; i++) {
        kq += i * hs[i] * pow(x, i - 1);
    }
    return kq;
}

// Hàm f''(x)
double daoham2_f(double x, int bac) {
    double kq = 0;
    for (int i = 2; i <= bac; i++) {
        kq += i * (i - 1) * hs[i] * pow(x, i - 2);
    }
    return kq;
}

// Thuật toán Newton
double newton(double a, double b, double x0, double eps, int maxlap, int bac) {
    double x = x0; // Điểm bắt đầu
    double saiso = 1e9; // Khởi tạo sai số 1 tỷ để vào vòng lặp, vì điều kiện sai số ban đầu cần > eps
    int lap = 0; // Biến đếm số lần lặp
    int found = 0; // Biến kiểm tra tìm thấy nghiệm

    printf("\n|------------|------------------|------------------|\n");
    printf("| LAN LAP    | x (x_k+1)        | SAI SO           |\n");
    printf("|------------|------------------|------------------|\n");

while (saiso > eps && lap < maxlap) {
    double fx = f(x, bac);
    double fdx = daoham_f(x, bac);

    if (x < a || x > b) {
    printf(">> Canh bao: Newton nhay ra ngoai khoang, dung lai tai x = %.6lf\n", x);
    break;
    }

    if (fabs(fdx) < 1e-12) {
        printf(">> Dao ham f'(x) = 0 tai x = %.6lf. Dung lai.\n", x);
        return x;
    }

    double x_sau = x - fx / fdx;
    saiso = fabs(x_sau - x);
    x = x_sau;
    lap++;

    double fx_sau = f(x, bac);
    // Nếu f(x) đã gần 0 thì gán sai số = 0 cho hiển thị đẹp
    if (fabs(fx_sau) < eps) saiso = 0.0;

    printf("| %-10d | %-16.10lf | %-16.10lf |\n", lap, x, saiso);

    if (fabs(fx_sau) < eps)
        break;
}
printf("|------------|------------------|------------------|\n");

if (saiso > eps && !found)
    printf(">> Khong tim thay nghiem trong khoang [%.3lf, %.3lf]\n", a, b);
else {
    printf(">> Nghiem gan dung: %.10lf\n", x);
    printf(">> Sai so: %.10lf\n", saiso);
    printf(">> So lan lap: %d\n", lap);
}
    return x;
}

// Kiểm tra hội tụ Furie
int furie(int bac, double a, double b) {
    // 1. Nếu đa thức bậc 1 thì luôn hội tụ
    if (bac == 1) return 1;
    // 2. Kiểm tra f'(x) không đổi dấu trên [a,b]
    int cungDau1 = daoham_f(a, bac) >= 0;
    int doiDau1 = 0;
    for (double i = a; i <= b; i += 0.01) {
        if ((daoham_f(i, bac) >= 0) != cungDau1) {
            doiDau1 = 1;
            break;
        }
    }

    // 3. Kiểm tra f''(x) không đổi dấu
    int cungDau2 = daoham2_f(a, bac) >= 0;
    int doiDau2 = 0;
    for (double i = a; i <= b; i += 0.01) {
        if ((daoham2_f(i, bac) >= 0) != cungDau2) {
            doiDau2 = 1;
            break;
        }
    }
    if (doiDau1 || doiDau2)
        printf(">> Canh bao: Dao ham doi dau tren [%.2lf, %.2lf], ket qua co the hoi tu cham hon.\n", a, b);


    //4. Chọn điểm bắt đầu x0
    if (f(a, bac) * daoham2_f(a, bac) > 0)
        return 1; // Dùng a làm x0
    else if (f(b, bac) * daoham2_f(b, bac) > 0)
        return -1; // Dùng b làm x0
    else
        return 0;
}


