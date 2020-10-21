#ifndef TODOLIST_H
#define TODOLIST_H

#include <QMainWindow>
#include<QMap>
#include<QPixmap>
#include<QString>
#include <QtSql>
#include <QList>

QT_BEGIN_NAMESPACE
namespace Ui {
class TodoList;
}
QT_END_NAMESPACE

// struct for the characters a character and a boolean to determine if the character is visible.
struct mychr {
  QString chr;
  bool show;
};

// the entire state of the game
struct state {
  QString category;
  QString phrase;
  QMap<int, mychr> parsedphrase;
  QString displayedphrase;
  bool gameover;
  int wrongguesses;
};

class TodoList : public QMainWindow {
  Q_OBJECT

 public:
  // constructor / destructor
  TodoList(QWidget* parent = nullptr);
  ~TodoList();

 private slots:
  // connected to the ui form buttons
  void on_RESET_clicked();
  void keyboardButtonPressed();

 private:
  Ui::TodoList* ui;
  // class variables
  static QSqlDatabase db;
  static QVector<QPixmap> imap;
  static state mstate;
  // class functions
  void enabledisableKeyboard(bool);
  void setDisplay(state);
  QVector<QPixmap> setImages();
  state clearstate(state);
  state gameWon(state);
  state gameLost(state);
  state checkletter(QString, state);
};
#endif // TODOLIST_H
