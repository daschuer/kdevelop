/* KDevelop QMake Support
 *
 * Copyright 2006 Andreas Pakulat <apaku@gmx.de>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 */

#include "parsetest.h"

QTEST_MAIN( ParseTest );

ParseTest::ParseTest( QObject* parent = 0 )
        : QObject( parent )
{}

ParseTest~ParseTest()
{}

void ParseTest::successSimpleProject()
{
    QFETCH( QString, project );
    bool ret = QMake::Driver::parseString( project );
    QVERIFY( ret );
}

void ParseTest::successSimpleProject_data()
{
    QTest::addColumn<QString>( "project" );
    QTest::newRow( "row1" ) << "\n";
}

void ParseTest::failSimpleProject()
{
    QFETCH( QString, project );
    bool ret = QMake::Driver::parseString( project );
    QVERIFY( ret );
}

void ParseTest::failSimpleProject_data()
{
    QTest::addColumn<QString>( "project" );
    QTest::newRow( "row1" ) << "";
}

void ParseTest::successFullProject()
{
    QFETCH( QString, project );
    bool ret = QMake::Driver::parseString( project );
    QVERIFY( ret );
}

void ParseTest::successFullProject_data()
{
    QTest::addColumn<QString>( "project" );
    QTest::newRow( "row1" ) << "#Comment\n";
    QTest::newRow( "row2" ) << "\n";
}

void ParseTest::failFullProject()
{
    QFETCH( QString, project );
    bool ret = QMake::Driver::parseString( project );
    QVERIFY( ret );
}

void ParseTest::failFullProject_data()
{
    QTest::addColumn<QString>( "project" );
    QTest::newRow( "row1" ) << "foo()";
    QTest::newRow( "row2" ) << "{";
}

// kate: space-indent on; indent-width 4; tab-width 4; replace-tabs on

