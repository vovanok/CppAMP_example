// Основной заголовочный файл C++ AMP
#include <amp.h>

// Заголовочные файлы для работы с векторами и вводом/выводом
#include <vector>
#include <iostream>

void printVector(std::vector<int> vec);

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

	// Печать векторов
	std::cout << "A: ";
	printVector(pA);

	std::cout << "B: ";
	printVector(pB);

	std::cout << "C: ";
	printVector(pC);

	std::cin.get();
}

// Функция печати вектора элементов типа int
void printVector(std::vector<int> vec) {
	for (int item : vec) {
		std::cout << item << " ";
	}
	std::cout << std::endl;
}
