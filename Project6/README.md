#Project6 - http server
> Homework project 6 for **MACS, OS101, [FUT](http://freeuni.edu.ge)** - 2016 Fall

#Introduction
პროექტი არის მარტივი http server ის იმპლემენტაცია. აქვს რეინჯის, ქეშის და cgi-bin სკრიპტების მხარდაჭერა.

###Developers
Project is Written by the group of developers:
- [გიორგი გულიაშვილი](https://github.com/dev1)
- [გივი ბერიძე](https://github.com/viceplayer)
- [ლუკა მაჭარაძე](https://github.com/lmach14)
- [თორნიკე ჟიჟიაშვილი](https://github.com/RS200MT)

###External Libs
პროექტს ჭირდება UT ბიბლიოტეკა.<br />
sudo apt-get update; sudo apt-get install uthash-dev<br />

ასევე საჭიროა gcc-6. <br />
sudo apt-get update &&
sudo apt-get install build-essential software-properties-common -y &&
sudo add-apt-repository ppa:ubuntu-toolchain-r/test -y &&
sudo apt-get update &&
sudo apt-get install gcc-snapshot -y &&
sudo apt-get update &&
sudo apt-get install gcc-6 g++-6 -y &&
sudo update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-6 60 --slave /usr/bin/g++ g++ /usr/bin/g++-6 &&
sudo apt-get install gcc-4.8 g++-4.8 -y &&
sudo update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-4.8 60 --slave /usr/bin/g++ g++ /usr/bin/g++-4.8;
<br />
შეამოწმეთ რომ ვერსია სწორია
gcc -v

#Usage
პროექტი კომპილირდება და ეშვება შემდეგნაირად.
```bash
sudo make; sudo ./http_server test_config
```

##Feature
```
Http server, აქვს კონფიგ ფაილის მხარდაჭერა, სადაც შესაძლებელია აღიწეროს 1 ან მეტი ვირტუალური ჰოსტი სხვადასხვა ან იგივე პორტებზე, აუცილებელია რომ ჰოსტები უნიკალურები იყვნენ თუნდაც სხვადასხვა პორტებზე უსმენდნენ.
http servers შეუძლია ფაილების ETag ის დათვლა და შესაბამისად მოქცევა თუკი იუზერს სწორი თეგი აქვს.
ასევე შეუძლია ფაილების რეინჯებად გაგზავნა(მოთხოვნისას).
და CGI სკრიპტების გაშვება.

```

#Project Structure

###Tree
```
Project6
├── cgi_bin.c
├── cgi_bin.h
├── config.c
├── config.h
├── etag_helper.c
├── etag_helper.h
├── http_helper.c
├── http_helper.h
├── logger.c
├── logger.h
├── main.c
├── Makefile
├── map_entry.h
├── processor.c
├── processor.h
├── README.md
├── server.c
├── server.h
├── string_helper.c
├── string_helper.h
├── test_config
├── url.c
└── url.h
```

###Modules

- **url.[c/h]**:
> url ის დეკოდირებისთვის(%ების და მისნაირების გადასათარგმნად) ვიყენებ სტეკ ოვერფლოუზე დადებულ კოდს. https://stackoverflow.com/questions/2673207/c-c-url-decode-library


- **string_helper.[c/h]**:
> იმპლემენტირებს ერთადერთ ფუნქციას რომელიც მთელ C სტრინგის ასოებს გადაუვლის და დააპატარავებს. ვიყენებ http ჰედერების და ველიუებისთვის.

- **map_entry.h**:

``` C
...
typedef struct config_map_entry {
    char *key;
    char *value;
    struct config_map_entry *sub;
    UT_hash_handle hh;
} config_map_entry;


typedef struct http_map_entry {
    char *key;
    char *value;
    UT_hash_handle hh;
} http_map_entry;
...
```
>კონფიგ ფაილს ვინახავ ორმაგ UT მეპში. პირველი მეპის key არის დომეინის სახელი, მეორე მეპის key კი კონფიგის ყველა მნიშვნელობა.
> http რექვესთს კი 1მაგ მეპში გარდავქმნი, მაგალითად content-length key ზე იქნება რამდენი სიგრძისაა რიცხვის სტრინგი.

- **http_helper.[c/h]**:

>http_helper ი პასუხისმგებელია, შმოსული რექვესთის პარსინგზე, ველიუების მარტივად ამოღებაზე და საბოლოოდ ყველა რესურსის გამონთავისუფლებაზე.

- **etag_helper.[c/h]**:

>etag_helper ი პასუხისმგებელია, ფაილის და სტრინგის მითითებულ ზომად დაჰეშვაზე. რეალურად etag_helperი წინასწარ დიფეინით განსაზღვრული სიგრძის ჰეშს ითვლის. თუკი მასზე გრძელს მოვითხოვთ რაღაც სიმბოლოთი შეავსებს და თუკი ნაკლებს უბრალოდ პრეფიქსს აიღებს. ჰეშის დასათვლელად ყოველ K ცალ სეგმენტს იღებს და ქსორავს. უბრალოდ ყველაზე მარტივი რამ დავაიმპლემენტირე.

- **config.[c/h]**:

> config ფაილი პასუიხსმგებელია პროგრამის გაშვებისას კონფიგის ერთხელ წაკითხვაზე შენახვაზე და ველიების დაბრუნებაზე. აღსანიშნავია რომ ეს მეპი გლობალურადაა ხელმისაწვდომი და ასევე ის რომ ის სტატიკურია წაკითხვის შემდეგ. UT მეპის ახალი ვერსიები შესაძლებელს ხდიან სტატიკურ მეპზე კონკრეტულად ვიკითხოთ ამიტომ ლოქები არ გვჭირდება. კონფიგი ასევე ქმნის ლოგ ფაილში ჩასაწერ ლოქს ყველა ჰოსტისთვის.

- **logger.[c/h]**:

> logger პასუხისმგებელია ჰოსტებში ლოგების სწორად ჩაწერაზე. თუკი ისეთი რექვესთი მოუვიდა რომლის შესაბამი ფაილსაც ვერ პოულობს, ან დომეინს, ან უბრალოდ ფაილს ვერ ხსნის, გამოიტანს კონსოლში.


- **main.c**:
> მეინი ამოწმებს რომ GCC ის ახალი კომპილატორით ვაკომპილირებთ. არეგისტრირებს გლობალურად კონფიგ ფაილს. ქმნის ვორქერ სრედებს.შემდეგ წერია კონფიგის გაუქმება, თუმცა ჩვენ სერვერს გამორთვის საფორთი არ აქვს ასე რომ კოდი აქამდე ვერ მივა.

- **processor.[c/h]**:

> პროცესორს აბსტრაქცია აქვს გაკეთებული epollზე. მისი ინითისას, იქმნება იპოლი და ვორქერ სრედები. ვორქერ სრედები მუდმივად ცდილობენ რამის ამოღებას იპოლიდან, ლისენერ სრედებს კი (რომლებიც სერვერ.ც) ში არიან მუდმივად შეუძლიათ ახალი ქონექშენების დამატება. როდესაც ვორქერი ახალ მზა რექვესთს ამოიღებს, ის გაუშვებს მის შესაბამის ფუნქციას, და ელოდება სანამ არ მორჩება, შემდეგ კი იგივე მეორდება. აღსანიშნავია რომ ყველა ივენთი ვარდება EPOLLONESHOT ფლეგით, ასე რომ ის ამოღებისთანავე ატომურად იშლება ოპოლიდან და შეიძლება მისი ჩამატება თავიდან გახდეს საჭირო.

- **cgi_bin.[c/h]**:

> cgi_bin ს შეუძლია შეამოწმოს არის თუ არა რექვესთი მისი. როგორც კლასრუმზე ითქვა თუკი .ცგი ფაილია ანუ ცგი-ბინ ის სკრიპტის ძებნა უნდა დავიწყოთ. ამის შემდეგ ვიფოკებით ორი პაიპის დახმარებით შედგება კონტაქტი სკრიპტსა და ჩვენ სერვერს შორის. 


- **server.[c/h]**:

> server ში მოთავსებულია ყველაზე დიდი ფუნქციონალი(სამწუხაროდ). აქ არის პორტზე ლისენერების კოდი, რომლებიც კონკურენტულა არიან გაშვებულები და ახალ კლიენტებს იპოლში ყრიან. ასევე არის ის კოდი რომელიც ეშვება ყველა მზა ქონექშენის ამოღებისას. საბოლოოდ ყველაფერი აქ ერთდება. ვიძახებთ http ის გაპარსვას, ვარკვევთ ცგი სკრიპტია თუ არა. საჭიროების შემთხვევაში ვითვლით ჰეშს, ვაგზავნით რეინჯებს ან ერორს.

###makefile
project is built using [make](http://www.gnu.org/software/make/manual/make.html),
file contains instructions for [make](http://www.gnu.org/software/make/manual/make.html)
to build project.

###README.md
readme of the project.
