/***************************************************************************
 *   Copyright (C) 2003 by Mario Scalas                                    *
 *   mario.scalas@libero.it                                                *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <qlineedit.h>
#include <qtextedit.h>
#include <qpushbutton.h>
#include <qtooltip.h>
#include <qlayout.h>

#include <klistview.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kdebug.h>

#include "partexplorerformbase.h"
#include "partexplorerform.h"

///////////////////////////////////////////////////////////////////////////////
// class PropertyItem
///////////////////////////////////////////////////////////////////////////////

class PropertyItem : public KListViewItem
{
public:
    PropertyItem( KListViewItem *parent, const QString &propertyName,
        const QString &propertyType, const QString &propertyValue )
        : KListViewItem( parent )
    {
        setText( 0, propertyName );
        setText( 1, propertyType );
        setText( 2, propertyValue );
    }

    QString tipText() const
    {
        QString tip = "Name: %1 | Type: %2 | Value: %3";
        return tip.arg( text(0) ).arg( text(1) ).arg( text(2) );
    }
};

///////////////////////////////////////////////////////////////////////////////
// class ResultsList
///////////////////////////////////////////////////////////////////////////////

class ResultsList : virtual public KListView, virtual public QToolTip
{
public:
    ResultsList( QWidget *parent )
        : KListView( parent, "resultslist" ), QToolTip( viewport() ) {}
    virtual ~ResultsList() {}

    virtual void maybeTip(const QPoint &p)
    {
        PropertyItem *item = dynamic_cast<PropertyItem*>( itemAt( p ) );
        if ( item )
        {
            QRect r = itemRect( item );
            if ( r.isValid() )
                tip( r, item->tipText() );
        }
    }

    void clear()
    {
        KListView::clear();
    }
};

///////////////////////////////////////////////////////////////////////////////
// class PartExplorerForm
///////////////////////////////////////////////////////////////////////////////

PartExplorerForm::PartExplorerForm( QWidget *parent )
    : PartExplorerFormBase( parent, "partexplorerform", 0 )
{
    resultsList = new ResultsList( this );
    resultsList->addColumn( tr2i18n( "Property" ) );
    resultsList->addColumn( tr2i18n( "Type" ) );
    resultsList->addColumn( tr2i18n( "Value" ) );
    resultsList->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)3,
        (QSizePolicy::SizeType)3, 0, 0,
        resultsList->sizePolicy().hasHeightForWidth() ) );
    PartExplorerFormBaseLayout->addWidget( resultsList );


    connect( typeEdit, SIGNAL(returnPressed()), this, SLOT(slotSearchRequested()) );
    connect( costraintsText, SIGNAL(returnPressed()), this, SLOT(slotSearchRequested()) );

    connect( searchButton, SIGNAL(clicked()), this, SLOT(slotSearchRequested()) );
}

///////////////////////////////////////////////////////////////////////////////

PartExplorerForm::~PartExplorerForm()
{
}

///////////////////////////////////////////////////////////////////////////////

QString PartExplorerForm::serviceType() const
{
    QString st = this->typeEdit->text();

    return st.isEmpty()? QString::null : st;
}

///////////////////////////////////////////////////////////////////////////////

QString PartExplorerForm::costraints() const
{
    QString c = this->costraintsText->text();
    return c.isEmpty()? QString::null : c;
}

///////////////////////////////////////////////////////////////////////////////

void PartExplorerForm::slotSearchRequested()
{
    QString serviceType = this->serviceType(),
        costraints = this->costraints();

    kdDebug(9000) << "===> PartExplorerForm::slotSearchRequested(): " <<
        " serviceType = " << serviceType << ", costraints = " << costraints << endl;

    if (serviceType.isNull() && serviceType.isNull())
    {
        slotDisplayError( i18n("You must fill at least a field!!") );
        return;
    }

    // Query for requested services
    KTrader::OfferList foundServices = KTrader::self()->query( serviceType, costraints );
    fillWidget( foundServices );
}

///////////////////////////////////////////////////////////////////////////////

void PartExplorerForm::slotDisplayError( QString errorMessage )
{
    if (errorMessage.isEmpty())
    {
        errorMessage = i18n("Unknown error!");
    }
    KMessageBox::error( this, errorMessage );
}

///////////////////////////////////////////////////////////////////////////////

void PartExplorerForm::fillWidget( const KTrader::OfferList &services )
{
    this->resultsList->clear();

    if ( services.isEmpty())
    {
        slotDisplayError( i18n("No service found matching the criteria!") );
        return;
    }

    this->resultsList->setRootIsDecorated( true );

    KListViewItem *rootItem = 0;

    KTrader::OfferList::ConstIterator it = services.begin();
    for ( ; it != services.end(); ++it )
    {
        KService::Ptr service = (*it);
        KListViewItem *serviceItem = new KListViewItem( this->resultsList, rootItem, service->name() );

        QStringList propertyNames = service->propertyNames();
        for ( QStringList::const_iterator it = propertyNames.begin(); it != propertyNames.end(); ++it )
        {
            QString propertyName = (*it);
            QVariant property = service->property( propertyName );
            QString propertyType = property.typeName();
            QString propertyValue;
            if (propertyType == "QStringList") {
                propertyValue = property.toStringList().join(", ");
            }
            else {
                propertyValue = property.toString();
            }

            QString dProperty = " *** Found property < %1, %2, %3 >";
            dProperty = dProperty.arg( propertyName ).arg( propertyType ).arg( propertyValue );
            kdDebug( 9000 ) << dProperty << endl;

            new PropertyItem( serviceItem, propertyName, propertyType, propertyValue );
        }
    }
}

#include "partexplorerform.moc"
