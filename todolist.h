#ifndef TODOLIST_H
#define TODOLIST_H

#include <QMainWindow>
#include<QMap>
#include<QPixmap>
#include<QString>
#include <QtSql>

QT_BEGIN_NAMESPACE
namespace Ui {
class TodoList;
}
QT_END_NAMESPACE
struct mychr {
  QString chr;
  bool show;
};
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
  TodoList(QWidget* parent = nullptr);
  ~TodoList();

 private slots:
  void on_RESET_clicked();
  void keyboardButtonPressed();

 private:
  Ui::TodoList* ui;
  static QSqlDatabase db;
  static QMap<int, QPixmap> imap;
  static state mstate;
  void enabledisableKeyboard(bool);
  void setDisplay(state);
  QMap<int, QPixmap> setImages();
  state clearstate(state);
  state getcatandphrase(state);
  state parsephrase(state);
  state gameWon(state);
  state gameLost(state);
  state checkletter(QString, state);
};
#endif // TODOLIST_H
