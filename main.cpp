#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <ctime>
#include <string.h>
#include <string>
#include <iostream>
#include <fstream>
#include <semaphore.h>

sem_t database_access; // семафор для ограничения доступа к базе данных
sem_t readers_changing; // семафор для смены количества читателей
sem_t information_output; // семафоры для вывода информации в консоль/файл
const int SIZE = 11; // размер базы данных 
int DB[SIZE];  // база данных
int readers_count = 0; // количество читателей, пользующихся библиотекой в какой-то конкретный момент
int readers_size; // количество читателей (всего)
int writers_size; // количество писателей (всего)
FILE *output; // файл для вывода информации

// метод для сортировки базы данных (в условии сказано, 
// что после изменений она должна сохранять непротиворечивое состояние)
void sort_database() {
   for (int i = 0; i < SIZE - 1; ++i) {
       for (int j = i + 1; j < SIZE; ++j) {
           if (DB[i] > DB[j]) {
               std::swap(DB[i], DB[j]);
           }
       }
   }
}

// метод, который превращает строку в неотрицательное число.
// если это невозможно, то выбрасывается исключение
int to_number(const std::string& s) {
    std::string out = "";
    for (auto ch : s) {
        if (ch < '0' || ch > '9') {
            throw std::invalid_argument("");
        }
        out += ch;
    }
    return std::stoi(out);
}

// функция потоков-читателей
void *funcRead(void *param) {
    int r_num = *(int *) param; // вытаскиваем номер читателя из входных параметров

    usleep(((rand() + clock()) * 1234567) % 5000000);

    // блокируем возможность менять количество читателей (чтоб другой поток не влез и не устроил дедлок)
    sem_wait(&readers_changing);
    readers_count++;
    // если пока нет читателей, имеющих доступ к библиотеке, то заблокируем его
    // если же есть, то ничего блокировать не нужно, т.к. может подключаться одновременно сколько угодно читателей.
    if (readers_count == 1) {
        sem_wait(&database_access);
    }
    // освобождаем возможность менять количество читателей
    sem_post(&readers_changing);

    // блокируем возможность записи в файл/консоль для конкурирующих потоков
    sem_wait(&information_output);
    // вывод информации о начале действий с базой данных конкретного читателя
    fprintf(output, "reader %d started his work with db\n", r_num);
    std::cout << "reader " << r_num << " started his work with db\n";
    // освобождаем возможность писать в файл/консоль
    sem_post(&information_output);
    
    usleep(((rand() + clock()) * 1234567) % 500000);
    
    // читаем рандомный элемент и выводим информацию об этом (аналогично блокируя потоки)
    int random_pos = (rand() + clock()) % SIZE;
    sem_wait(&information_output);
    fprintf(output, "READER %d READ %dTH ELEMENT. IT IS %d\n", r_num, random_pos + 1, DB[random_pos]);
    std::cout << "READER " << r_num << " READ " << random_pos + 1 << "TH ELEMENT. IT IS " << DB[random_pos] << '\n';
    sem_post(&information_output);
        
    usleep(((rand() + clock()) * 1234567) % 500000);    
    
    // выводим информацию об окончании конкретного читателя действий с базой данных
    sem_wait(&information_output);
    fprintf(output, "reader %d finished his work with db\n", r_num);
    std::cout << "reader " << r_num << " finished his work with db\n";
    sem_post(&information_output);

    // блокируем возможность изменять количество читателей
    sem_wait(&readers_changing);
    readers_count--;
    // если это был последний читатель, то освобождаем доступ к базе данных
    if (readers_count == 0) {
        sem_post(&database_access);
    }
    // освобождаем возможность изменять количество читателей
    sem_post(&readers_changing);

    return nullptr;
}

// функция потоков-писателей
void *funcWrite(void *param) {
    int w_num = *(int *) param; // вытаскиваем номер читателя из входных параметров

    usleep(((rand() + clock()) * 1234567) % 5000000);

    // блокируем доступ к базе данных при первой возможности
    sem_wait(&database_access);
    
    // предполагаю, что решение правильное, поэтому конкурирующих потоков на запись у этого потока быть не должно,
    // тогда и блокировать их не будем

    // вывод информации о начале действий с базой данных
    fprintf(output, "writer %d started his work with db\n", w_num);
    std::cout << "writer " << w_num << " started his work with db\n";
    
    usleep(((rand() + clock()) * 1234567) % 500000);
    
    // писатель случайным образом меняет записи в базе данных 
    int random_pos = (rand() + clock()) % SIZE;
    int prev_number = DB[random_pos];
    int new_number = (rand() + clock()) % 20;
    DB[random_pos] = new_number;
    // вывод информации об успешной смене записи писателем
    fprintf(output, "WRITER %d CHANGED %dTH ELEMENT IN DATABASE FROM %d TO %d\ndatabase was also sorted\n", 
        w_num, random_pos + 1, prev_number, new_number);
    std::cout << "WRITER " << w_num << " CHANGED " << random_pos + 1 << "TH ELEMENT IN DATABASE FROM ";
    std::cout << prev_number << " TO " << new_number << "\ndatabase was also sorted\n";
    // возврат базы данных к непротиворечивому виду (необходимо по условию)
    sort_database();
    
    usleep(((rand() + clock()) * 1234567) % 500000);
    
    // вывод информации об окончании действий с базой данных
    fprintf(output, "writer %d finished his work with db\n", w_num);
    std::cout << "writer " << w_num << " finished his work with db\n";

    // освобождаем доступ к базе данных 
    sem_post(&database_access);

    return nullptr;
}

int main(int argc, char *argv[]) {
    // ввод данных 
    if (argc > 2) {
        std::cout << "wrong input\n";
        return 0;
    }
    if (argc == 1) {
        readers_size = (rand() + clock() * 1234567) % 11;
        writers_size = (rand() + clock() * 1234567) % 11;
        std::cout << "random numbers was generated\n";
        std::cout << "number of readers: " << readers_size << '\n';
        std::cout << "number of writers: " << writers_size << "\n\n";
    } else if (strcmp(argv[1], "console") == 0) {
        std::string a, b;
        std::cout << "input format: \"<number of readers> <number of writers>\"\n";
        try {
            std::cin >> a >> b;
            readers_size = to_number(a);
            writers_size = to_number(b);
        } catch (...) {
            std::cout << "wrong input\n";
            return 0;
        }
    } else {
        try {
            std::ifstream fin(argv[1]);
            std::string a, b;
            fin >> a >> b;
            readers_size = to_number(a);
            writers_size = to_number(b);
        } catch (...) {
            std::cout << "wrong filename or input\n";
            return 0;
        }
    }

    output = fopen("output.txt", "w");

    // инициализация семафоров
    sem_init(&database_access, 0, 1);
    sem_init(&readers_changing, 0, 1);
    sem_init(&information_output, 0, 1);

    // заполнение начальной базы данных рандомными данными
    std::cout << "initial database: \n";
    for (int i = 0; i < SIZE; ++i) {
        DB[i] = i + 1;
        std::cout << DB[i] << ' ';
    }
    std::cout << "\n\n";

    //создание потоков-писателей
    pthread_t threadW[writers_size];
    int writers[writers_size];
    for (int i = 0; i < writers_size; i++) {
        writers[i] = i + 1;
        pthread_create(&threadW[i], nullptr, funcWrite, &writers[i]);
    }

    //создание потоков-читателей
    pthread_t threadR[readers_size];
    int readers[readers_size];
    for (int i = 0; i < readers_size; ++i) {
        readers[i] = i + 1;
        pthread_create(&threadR[i], nullptr, funcRead, &readers[i]);
    }

    // соединение потоков в конце, чтоб программа не завершилась раньше дочерних потоков
    for (int i = 0; i < writers_size; ++i) {	
        pthread_join(threadW[i], nullptr);
    }

    for (int i = 0; i < readers_size; ++i) {
        pthread_join(threadR[i], nullptr);
    }

    // подчистка семафоров
    sem_destroy(&database_access);	
    sem_destroy(&readers_changing);
    sem_destroy(&information_output);
    
    // вывод базы данных после всех произведенных с ней действий
    std::cout << "\n\nnew database: \n";
    for (int i = 0; i < SIZE; ++i) {
        std::cout << DB[i] << ' ';
    }
    std:: cout << "\n\n";

    return 0;
}
