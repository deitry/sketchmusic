# sketchmusic
Musical some-kind-of-notation editor for UWP with built-in SF2 player written in C++/CX

It is divided into two major parts:
- SketchMusic is library which provides all the types used in the application;
- StrokeEditor - which is actual app that uses SketchMusic library.

Original intent was to develop such app that could be used to create music through three stages:
1. Quick record of some musical idea - with possibility of subsequent modification.
2. Work with database of musical ideas - in order to find the best and improve them.
3. Arrange ideas into whole music composition!

What is done by now:
- It is possible to create a musical idea, edit and replay it with the use of built-in player (based on SF2 soundfonts).
- All ideas are stored in single database (using SQLite), can be accessed on it's own screen, filtered, arranged by rating or whatever.
- It is possible to create composition (which may be considered as grand-idea) and work on it, but it is highly unstable feature.
Compositions are stored in their own files.
- - Look through sources if you want to learn more details

What is should be done (there's a lot of issues, but this is most wanted):
- Export idea to conventional formats such as MIDI
- BUGFIX. It crashes all the time. I considered this app development as kind of training in order to improve my programming skills
(and occasionally build helpful application), so there's A LOT of things I later realized was design errors.
(saving the mistakes I didn't tracked).

- - - - - - - - - - - - - - - - - - -

Repo contains (I guess) all files that could be needed to build and launch the app including soundfonts I found in the Internet.

Since I am only developer, all comments in code use my native Russian language, so it may be difficult
to not-Russian speakers to understand what's going on. If there will be such necessity, I can explain the details in English.

If you somehow interested in this app, feel free to contact me: dm.s.vornychev@gmail.com Dmitriy
