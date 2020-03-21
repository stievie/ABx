# Contributing

If you want to contribute to this project, please fork the repository and to
submit pull requests.

If you have questions you can reach me via Email with the Email address shown in
my profile.

## Rules

* The license is MIT and submitted code must not be incompatible with it.
* This project is written in C++ using the C++17 standard, and may switch to C++20
one time.
* You are encouraged to use new C++ features. Partly this project is also a playground
to explore modern C++.
* The code should be readable and it should be obvious what it does. Don't waste 
time with micro optimizations which make the code unreadable, the compiler will
optimize it much better anyways.
* Please stick to the coding style.
* Use Getters and Setters only when they make sense. If you just want read and
write access to a member variable, make the variable public.

## Windows

On Windows the easiest is to use Visual Studio 2019 (Community). For each project
there is a solution file `.sln` in the respective directory. There is also
a solution file (`absall/absall.sln`) which contains all server projects.

## Linux

As C++ IDE on Linux I prefer QtCreator. There are QtCreator project
files `.creator` in the subdirectories of the projects. You can also setup
QtCreator to build the projects with the makefiles in the `makefiles` directory.
