#ifndef DBASEADDNUMBER_H
#define DBASEADDNUMBER_H

#include "dbasevisitor.h"
#include "dbasecomando.h"

#include <QDebug>

class dBaseADDnumber: public dBaseComando {
public:
    dBaseADDnumber() { qDebug() << "dBaseADDnumber"; }
    virtual void accept(class dBaseVisitor &v);
    float getNumber(void) { return 42.2; }
};

#endif // DBASEADDNUMBER_H
