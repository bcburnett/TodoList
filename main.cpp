// standard qt creator boilerplate main.cpp
#include "todolist.h"
#include <QApplication>
int main(int argc, char* argv[]) {
  QApplication a(argc, argv);
  TodoList w;
  w.setWindowTitle("Letter Guess Game");
  w.show();
  return a.exec();
}
