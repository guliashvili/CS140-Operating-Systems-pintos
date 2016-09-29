#Project1 - free shell
> Homework project 1 for **MACS, OS101, [FUT](http://freeuni.edu.ge)** - 2016 Fall

#Introduction
პროექტი არის Free shell_ის იმპლემენტაცია. მას აქვს რამდენიმე ბილთინ ფუნქციონალი და ასევე შეუძლია სხვა პროგრამების გაშვებაც.

###Developers
Project is Written by the group of developers:
- [გიორგი გულიაშვილი](https://github.com/dev1)
- [გივი ბერიძე](https://github.com/viceplayer)
- [ლუკა მაჭარაძე](https://github.com/lmach14)
- [თორნიკე ჟიჟიაშვილი](https://github.com/RS200MT)

###External Libs
პროექტი იყენებს დამატებით ბიბლიოთეკას readline_ს.
მის დაყენებას ჭირდება შემდეგი ინსტრუქციები.
```bash
sudo apt-get update
sudo apt-get install libreadline6 libreadline6-dev
```

#Usage
დაკომპილირებული პროექტი ეშვება შემდეგნაირად.
```bash
sudo ./fsh
```

##Feature
Free shell, built-in ფუნქციებად იმპლემენტირებს ?, cd, exit, kill, pwd, ulimit, nice, echo, type, export ფუნქციებს, ასევე შეუძლია კონკრეტული გაშვებადი ფაილების გაშვება, ან/და ფაილების მოძებნა PATH გარემოს ცვლადის მიერ მითიტებულ დირექტორიებში. ბრძანებები შეიძლება დაკავშირდეს ლოგიკური ან, და ბრძანებებით. ასევე შესაძლებელია მათი სრული სეპარაცია ; ბრძანებით, მარჯვენა ბრძანების აუთფუთის, მარცხენა ბრძანების ინფუთში გადამისამართება.
ასევე შესაძლებელია რაიმე ფაილის ინფუთის ბრძანების ინფუთად გადამისამართება, ან ბრძანების აუთფუთის ფაილზე მიწეპება ან გადაწერა.

```

#Project Structure

###Tree
```
.
├── built_in_functions
│   ├── about.c
│   ├── about.h
│   ├── cd.c
│   ├── cd.h
│   ├── echo.c
│   ├── echo.h
│   ├── execute.c
│   ├── execute.h
│   ├── execute_path.c
│   ├── execute_path.h
│   ├── exit.c
│   ├── exit.h
│   ├── export.c
│   ├── export.h
│   ├── kill.c
│   ├── kill.h
│   ├── nice.c
│   ├── nice.h
│   ├── pwd.c
│   ├── pwd.h
│   ├── type.c
│   ├── type.h
│   ├── ulimit.c
│   ├── ulimit.h
│   ├── utility.c
│   └── utility.h
├── controller.c
├── controller.h
├── main.c
├── Makefile
├── parser.c
├── parser.h
└── README.md
```

###Modules

- **controller.[c/h]**:
> კონტროლერი პასუხისმგებელია გაპარსული ხაზის, გაშვებაზე, ინფუთ/აუთფუთის სწორად გადამისამართებაზე, ზოგიერთი ფუნქციის გაშვებამდე დაფორკვაზე და ლოგიკური ოპერაციების კონტროლზე.
> ქომანდების ერთმანეთზე ინფუთ აუთფუთის გადამისამართება ყოველთვის ხდება პაიპით. ბილთინ პროგრამებს შორის კავშირი უფრო სწრაფი მეთოდითაც შეიძლებოდა, თუმცა performanceს, ნაკლები კოდის/შეცდომების წერა ვარჩიეთ. 

- **parser.[c/h]**:
> პარსერი პარსავს იუზერის მიერ შემოყვანილ ხაზებს, აცალკევებს დამოუკიდებელ ბრძანებებს და აბრუნებს უფრო მოსახერხებელ სტრუქტურებს, რომლებზე წვდომაც შესაძლებელია იტერატორის მსგავსი მეთოდით.

``` C
...
typedef enum command_linkage {
    AND, OR, ANYWAY, PIPE
} command_linkage;

typedef struct split_commands_info {
    char **commands;
    command_linkage *linkages;

    int commands_N;
} split_commands_info;

typedef struct command_explained {
    char *command;
    char **command_parameters;
    char *file_to_overwrite;
    char *file_to_read;
    char *file_to_append;

    int it;
} command_explained;
...
```
>შემოსული ხაზი იჩეხება ცალკეულ ბრძანებებად. დაჩეხვა ხდება &&,||,|,; ოპერატორებზე.

- **main.c**:
> მეინი აუქმებს ctrl-c, ctrl-z სიგნალებზე გაშვებას, ამოწმებს თუ არის ბრძანება გადმოცემული არგუმენტად, თუ არადა სამუდამო ციკლით იღებს და ინახავს იუზერის ინფუთს readline ბიბლიოთეკის დახმარებით. #undef NDEBUG ის დაწერის შემთხვევაში მეინი ასევე გაუშვებს პროექტის შიდა ტესტებს.

###makefile
project is built using [make](http://www.gnu.org/software/make/manual/make.html),
file contains instructions for [make](http://www.gnu.org/software/make/manual/make.html)
to build project.

###README.md
readme of the project.
