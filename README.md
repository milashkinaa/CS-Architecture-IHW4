# __Индивидуальное домашнее задание 4 по дисциплине "Архитектура вычислительных систем"__

# __Алёшкина Милана, БПИ216__

__Вариант 4. Условие задачи:__ Задача о читателях и писателях. Базу данных разделяют два типа процессов – читатели и писатели. Читатели выполняют транзакции, которые
просматривают записи базы данных, транзакции писателей и просматривают
и изменяют записи. Предполагается, что в начале БД находится в непротиворечивом состоянии (например, если каждый элемент — число, то они все отсортированы). Каждая отдельная транзакция переводит БД из одного непротиворечивого состояния в другое. Для предотвращения взаимного влияния
транзакций процесс-писатель должен иметь исключительный доступ к БД.
Если к БД не обращается ни один из процессов-писателей, то выполнять транзакции могут одновременно сколько угодно читателей. 

Создать многопоточное приложение с потоками-писателями и потоками-читателями. Реализовать решение, используя семафоры.

__Как работать с программой:__
Для ввода данных из консоли нужно передать в параметры командной строки console, для ввода из файла - ввести имя файла, для генерации случайных чисел можно ничего не передавать.

__Модель параллельного программирования__, подходящая под условие задачи - взаимодействующие равные. Главного потока нет, есть переменная, доступ к которой есть как у читательских потоков, так и у одного из писательских. 

[__Тесты собраны в этой папке__](https://github.com/milashkinaa/CS-Architecture-IHW4/tree/main/tests)

__Описание алгоритма:__ 
Есть база данных, у которой есть читатели и писатели. Читатели могут только просматривать эти записи, а писатели могут эти записи изменять. Чтобы все писатели одновременно не меняли записи, одновременно доступ к БД может быть предоставлен только одному из писателей. Пока один писатель пишет, читатели и остальные писатели не могут подключиться к БД. При этом читать записи могут сколько угодно читателей. При этом писатели в момент чтения изменить ничего не могут.

Семафоров несколько:

1. Первый семафор отвечает за вывод информации. У нас несколько потоков в файле, одновременно работать они работать не могут, выводиться одновременно информация тоже не может. Как раз за соблюдение этих условий и отвечает данный семафор.

2. Второй семафор отвечает за количество читателей - он блокируют смену количества активных читателей. 

3. Третий семафор отвечает за доступ к базе данных. Писатели запрашивают доступ и ждут, пока текущий писатель закончит свои действия, либо читатели закончат читать. Читатели же сначала ждут возможность измены количества читателей

![Вот так вот]( https://sun9-14.userapi.com/impg/d7X2BONunlIASqUJ8l6vPHhb73_QN9KGqAXibA/zx0cejGPZZc.jpg?size=604x604&quality=96&sign=9c2ebb1f41118fab8ab709324c28a8e2&type=album "Вот так вот")
