# Contributing

If you want to contribute to this project, please fork the repository and
submit pull requests.

If you have questions, you can reach me via Email with the Email address shown in
my profile. You can also join the `#abxgameserver` IRC channel on the Freenode
network.

## Pull requests

The normal way to make Pull Requests (PR) is to create a separate feature branch:
~~~
git branch my-awesome-feature
~~~

Then commit your changes to this newly created branch and push it to your fork.
GitHub will asks you if you want to create a PR.

After your PR has been merged you can delete this feature branch using Github.

## Rules

* The license is MIT and submitted code must not be incompatible with it.
* The code must compile without warnings, or the CI build fails. The CI build also
fails when a test fails.
* You are encouraged to use new C++ features. Partly this project is also a playground
to explore modern C++.
* The code should be readable and it should be obvious what it does.
* Please stick to the coding style. There is a `.clang-format` file in the root directory,
you could just run it with `clang-format -style=.clang-format [<file> ...]`.

## Windows

On Windows the easiest is to use Visual Studio 2019 (Community). For each project
there is a solution file (`.sln`) in the respective directory. There is also
a solution file (`absall/absall.sln`) which contains all server projects.

## Linux

As C++ IDE on Linux I prefer QtCreator. QtCreator has excellent support for CMake,
so you can just open the top level `CMakeLists.txt` file with it.

![QtCreator](/Doc/qtcreator.png?raw=true)

Then you can configure QtCreator to build the project for you with GNU make or,
like in my case, Ninja.

![QtCreator Build](/Doc/qtcreator_build.png?raw=true)

## If you don't want to code

That's also great, because a game doesn't need only code. In fact coding is the
smallest problem. It also needs people who do:

* 3D Modeling
* Animations
* Story writing
* Creating maps
* Creating skills
* Music
* Sound effects
* Particle effects
* Icons
* Concept art

and a whole lot more.
