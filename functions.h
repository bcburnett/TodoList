#ifndef FUNCTIONS_H
#define FUNCTIONS_H
#include<QMap>
#include<QPixmap>
#include<QString>
#include <QtSql>
QMap<int, QPixmap> setImages();
struct mychr {
  QString chr;
  bool show;
};
struct state {
  QString category;
  QString phrase;
  QMap<int, mychr> parsedphrase;
  QString displayedphrase;
  bool gamewon;
  bool gamelost;
  int wrongguesses;
  bool dbfail;
};
state checkletter(QString, state);
state getcatandphrase(state);
#endif // FUNCTIONS_H
