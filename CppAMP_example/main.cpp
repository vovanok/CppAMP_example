// Основной заголовочный файл C++ AMP
#include <amp.h>

// Заголовочные файлы для работы с векторами и вводом/выводом
#include <vector>
#include <iostream>

void printVector(std::vector<int> vec);
void printMatrix(std::vector<int> matrix, int n, int m);

int main() {

	// Получение списка всех устройcтв, поддерживающих C++ AMP
	std::vector<concurrency::accelerator> allDevices = concurrency::accelerator::get_all();

	// Вывод описания всех устройств, поддерживающих C++ AMP
	std::cout << "All devices descriptions:" << std::endl;
	for (concurrency::accelerator device : allDevices) {
		std::wcout << "- " << device.description << std::endl;
	}
	std::cout << std::endl;

	// Установка первого устройства в качестве устройтсва по умолчанию
	std::wcout << "Set device as default: " << allDevices[0].description << std::endl;
	concurrency::accelerator::set_default(allDevices[0].device_path);
	std::cout << std::endl;

	// Пример сложения векторов
	int n = 10;				// Размер векторов для сложения
	std::vector<int> pA(n);	// Первый складываемый вектор
	std::vector<int> pB(n);	// Второй складываемый вектор
	std::vector<int> pC(n);	// Результат сложения

	// Иницализация складываемых векторов реальными данными
	for (int i = 0; i < n; i++) {
		pA[i] = i;
		pB[i] = i * 2;
	}

	// При конструировании array_view происходит копирование данных
	// При вызове деструкторов array_view не происходит копирование памяти с устройтсва на хост из-за const (это и не нужно)
	concurrency::array_view<const int, 1> a(pA); // Первый складываемый вектор в памяти устройства
	concurrency::array_view<const int, 1> b(pB); // Второй складываемый вектор в памяти устройства
	concurrency::array_view<int, 1> c(pC); // Результат сложения векторов в памяти устройства
	c.discard_data(); // Благодаря этому не происходит лишнее копирование pC в память устройтва

	// Запуск параллельной функции для каждого потока из c.extent, помеченого индексом idx
	concurrency::parallel_for_each(c.extent, [=](concurrency::index<1> idx) restrict(amp) {
		c[idx] = a[idx] + b[idx];
	});

	// Копирование данных из памяти устройства в память хоста
	c.synchronize();

	std::cout << "ADD VECTORS" << std::endl;

	// Печать векторов
	std::cout << "A: ";
	printVector(pA);

	std::cout << "B: ";
	printVector(pB);

	std::cout << "C: ";
	printVector(pC);

	// Пример сложения матриц
	// Размеры матриц
	int nX = 10;
	int nY = 10;

	// Одномерные массивы, представляющие мастрицы по строкам
	std::vector<int> matA(nX * nY);
	std::vector<int> matB(nX * nY);
	std::vector<int> matRes(nX * nY);

	// Заполнение матриц данными
	for (int i = 0; i < nX; i++) {
		for (int j = 0; j < nY; j++) {
			matA[i + j * n] = i + j;
			matB[i + j * n] = i + 2 * j;
		}
	}

	// Создание двухмерных отображений матриц на память устройтсва
	// При вызове деструкторов array_view не происходит копирование памяти с устройтсва на хост из-за const (это и не нужно)
	concurrency::array_view<const int, 2> matA_dev(nX, nY, matA);
	concurrency::array_view<const int, 2> matB_dev(nX, nY, matB);
	concurrency::array_view<int, 2> matRes_dev(nX, nY, matRes);
	matRes_dev.discard_data(); // Благодаря этому не происходит лишнее копирование matRes в память устройтва

	// Запуск параллельно выполняющейся анонимной функции, заданной лямбда-выражением,
	// на эктенте, представляющим двухмерное множество потоков
	concurrency::parallel_for_each(matRes_dev.extent, [=] (concurrency::index<2> ind) restrict(amp) {

		// Можно использовать matRes_dev[ind], matA_dev[ind] и matB_dev[ind], но так нагляднее
		matRes_dev(ind[0], ind[1]) = matA_dev(ind[0], ind[1]) + matB_dev(ind[0], ind[1]);
	});

	// Копирование данных из памяти устройства в память хоста
	matRes_dev.synchronize();

	std::cout << std::endl << "ADD MATRIXES" << std::endl;

	// Пячать матриц
	std::cout << "Matrix A:" << std::endl;
	printMatrix(matA, nX, nY);

	std::cout << "Matrix B:" << std::endl;
	printMatrix(matB, nX, nY);

	std::cout << "Matrix Res:" << std::endl;
	printMatrix(matRes, nX, nY);

	std::cin.get();
}

// Функция печати вектора элементов типа int
void printVector(std::vector<int> vec) {
	for (int item : vec) {
		std::cout << item << " ";
	}
	std::cout << std::endl;
}

// Функция печати матрицы типа int зажданных размеров n x m
void printMatrix(std::vector<int> matrix, int n, int m) {
	for (int i = 0; i < n; i++) {
		for (int j = 0; j < m; j++) {
			std::cout << matrix[i + j * n] << "\t";
		}
		std::cout << std::endl;
	}
}
