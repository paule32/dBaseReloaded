#ifndef DBASECOMANDO_H
#define DBASECOMANDO_H

#include "dbasevisitor.h"
#include <QDebug>

class dBaseComando {
public:
    dBaseComando() { qDebug() << "dBaseComando"; }
    virtual void accept(class dBaseVisitor &v) = 0;
};

#endif // DBASECOMANDO_H
