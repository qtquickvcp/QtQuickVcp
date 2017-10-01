/****************************************************************************
**
** Copyright (C) 2017 Alexander Rössler
** License: LGPL version 2.1
**
** This file is part of QtQuickVcp.
**
** All rights reserved. This program and the accompanying materials
** are made available under the terms of the GNU Lesser General Public License
** (LGPL) version 2.1 which accompanies this distribution, and is available at
** http://www.gnu.org/licenses/lgpl-2.1.html
**
** This library is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
** Lesser General Public License for more details.
**
** Contributors:
** Alexander Rössler <alexander AT roessler DOT systems>
**
****************************************************************************/

import QtQuick 2.5
import QtQuick.Controls 1.4
import QtQuick.Layouts 1.3
import Machinekit.Application 1.0

ApplicationItem {
    property var toolTable: status.io.toolTable
    property color modifiedColor: "#FFFF99"
    property color errorColor: "#FF9999"

    readonly property bool errored: d.verifyToolId(d.updatedToolTable)
    readonly property alias modified: d.modified
    readonly property bool itemSelected: table.selection.count > 0

    id: root
    width: 800
    height: 600

    onToolTableChanged: d.updateToolTableCopy(toolTable)

    function addRow() { d.addRow(); }
    function removeRow() { d.deleteRow(); }
    function resetModifications() { d.updateToolTableCopy(root.toolTable); }
    function updateToolTable() { d.commitToolTable(); }

    QtObject {
        id: d
        readonly property int toolIdColumnWidth: 60
        readonly property int offsetColumnWidth: 45
        readonly property int parameterWidth: 90
        readonly property int commentWidth: 160
        readonly property int minimumToolId: 0
        readonly property int maximumToolId: 99999
        readonly property int minimumPocketId: 1
        readonly property int maximumPocketId: 99999
        readonly property int minimumPosition: 0
        readonly property int maximumPosition: 9

        property var updatedToolTable: []
        property var changeMatrix: []
        property var toolIdErrors: []
        property bool modified: false

        function updateToolTableCopy(origin) {
            d.updatedToolTable = JSON.parse(JSON.stringify(origin));
            d.updateChangeMatrix(origin);

            var newToolIdErrors = [];
            for (var x in origin) {
                newToolIdErrors.push(false);
            }
            toolIdErrors = newToolIdErrors;

            d.modified = false;
        }

        function updateChangeMatrix(origin) {
            var newChangeMatrix = [];
            for (var i = 0; i < origin.length; ++i) {
                var row = []
                for (var j = 0; j < table.columnCount; ++j) {
                    row.push(false);
                }
                newChangeMatrix.push(row);
            }
            d.changeMatrix = newChangeMatrix;
        }

        function updateToolData(row, column, data) {
            var role = table.getColumn(column).role;
            var oldData = updatedToolTable[row][role];
            if (data != oldData) {
                updatedToolTable[row][role] = data;
                updatedToolTable = updatedToolTable; // trigger update
                markCellAsModified(row, column);
            }
        }

        function updateAxisToolData(row, column, data, axis) {
            var role = table.getColumn(column).role;
            var oldData = updatedToolTable[row][role][axis];
            if (data != oldData) {
                updatedToolTable[row][role][axis] = data;
                updatedToolTable = updatedToolTable; // trigger update
                markCellAsModified(row, column);
            }
        }

        function markCellAsModified(row, column) {
            d.changeMatrix[row][column] = true;
            d.changeMatrix = d.changeMatrix; // trigger update
            d.modified = true;
        }

        function verifyToolId(data) {
            var error = false;

            var newToolIdErrors = new Array(data.length);
            for (var i in data) {
                for (var j = 0; j < i; ++j) {
                    if (data[i]["id"] == data[j]["id"]) {
                        newToolIdErrors[i] = true;
                        newToolIdErrors[j] = true;
                        error = true;
                    }
                }
            }
            toolIdErrors = newToolIdErrors;
            return error;
        }

        function prepareUpdatedToolTable() {
            for (var i in d.updatedToolTable) {
                var offset = d.updatedToolTable[i]['offset'];
                offset['x'] = offset[0];
                offset['y'] = offset[1];
                offset['z'] = offset[2];
                offset['a'] = offset[3];
                offset['b'] = offset[4];
                offset['c'] = offset[5];
                offset['u'] = offset[6];
                offset['v'] = offset[7];
                offset['w'] = offset[8];
            }
        }

        function selectRow(row) {
            table.selection.clear();
            table.selection.select(row);
        }

        function addRow() {
            var newRow = {}
            newRow["id"] = 0;
            newRow["pocket"] = 1;
            newRow["offset"] = { 0: 0, 1: 0, 2: 0, 3: 0, 4: 0, 5: 0, 6: 0, 7: 0, 8: 0 }
            newRow["diameter"] = 0.0;
            newRow["frontangle"] = 0.0
            newRow["backangle"] = 0.0;
            newRow["orientation"] = 0;
            newRow["comment"] = "";
            d.updatedToolTable.push(newRow);
            d.updateChangeMatrix(d.updatedToolTable);
            d.updatedToolTable = d.updatedToolTable;
            d.modified = true;
        }

        function deleteRow() {
            if (table.selection.count === 0) {
                return;
            }

            table.selection.forEach( function removeRow(row) {
                root.forceActiveFocus();
                d.updatedToolTable.splice(row, 1);
                d.updatedToolTable = d.updatedToolTable;
            });
            d.modified = true;

            if (table.rowCount > 0) {
                table.selection.select(table.rowCount - 1);
            }
        }

        function commitToolTable() {
            prepareUpdatedToolTable();
            root.command.updateToolTable(d.updatedToolTable);
            root.command.loadToolTable();
        }
    }

    TableView {
        id: table
        anchors.fill: parent

        model: d.updatedToolTable
        itemDelegate: null

        TableViewColumn {
            role: "id"
            title: qsTr("Tool ID")
            width: d.toolIdColumnWidth
            delegate: toolIdEdit
        }

        TableViewColumn {
            role: "pocket"
            title: qsTr("Pocket")
            width: d.toolIdColumnWidth
            delegate: pocketEdit
        }

        TableViewColumn {
            role: "offset"
            title: qsTr("X")
            width: d.offsetColumnWidth
            delegate: offsetEdit
        }

        TableViewColumn {
            role: "offset"
            title: qsTr("Y")
            width: d.offsetColumnWidth
            delegate: offsetEdit
        }

        TableViewColumn {
            role: "offset"
            title: qsTr("Z")
            width: d.offsetColumnWidth
            delegate: offsetEdit
        }

        TableViewColumn {
            role: "offset"
            title: qsTr("A")
            width: d.offsetColumnWidth
            delegate: offsetEdit
        }

        TableViewColumn {
            role: "offset"
            title: qsTr("B")
            width: d.offsetColumnWidth
            delegate: offsetEdit
        }

        TableViewColumn {
            role: "offset"
            title: qsTr("C")
            width: d.offsetColumnWidth
            delegate: offsetEdit
        }

        TableViewColumn {
            role: "offset"
            title: qsTr("U")
            width: d.offsetColumnWidth
            delegate: offsetEdit
        }

        TableViewColumn {
            role: "offset"
            title: qsTr("V")
            width: d.offsetColumnWidth
            delegate: offsetEdit
        }

        TableViewColumn {
            role: "offset"
            title: qsTr("W")
            width: d.offsetColumnWidth
            delegate: offsetEdit
        }

        TableViewColumn {
            role: "diameter"
            title: qsTr("Diameter")
            width: d.parameterWidth
            delegate: doubleEdit
        }

        TableViewColumn {
            role: "frontangle"
            title: qsTr("Front Angle")
            width: d.parameterWidth
            delegate: doubleEdit
        }

        TableViewColumn {
            role: "backangle"
            title: qsTr("Back Angle")
            width: d.parameterWidth
            delegate: doubleEdit
        }

        TableViewColumn {
            role: "orientation"
            title: qsTr("Orientation")
            width: d.parameterWidth
            delegate: orientationEdit
        }

        TableViewColumn {
            role: "comment"
            title: qsTr("Comment")
            width: d.commentWidth
            delegate: textEdit
        }
    }

    Component {
        id: toolIdEdit

        Rectangle {
            readonly property bool hasChanged: (d.changeMatrix[styleData.row] && d.changeMatrix[styleData.row][styleData.column]) ? true : false
            readonly property bool hasError: d.toolIdErrors[styleData.row] ? true : false
            color: hasError ? root.errorColor : (hasChanged ? root.modifiedColor : "transparent")

            TextInput {
                anchors.fill: parent
                text: styleData.value
                color: styleData.textColor
                validator: IntValidator { bottom: d.minimumToolId; top: d.maximumToolId }

                onEditingFinished: d.updateToolData(styleData.row, styleData.column, Number(text));
                onActiveFocusChanged: if (activeFocus) { d.selectRow(styleData.row); }
            }
        }
    }

    Component {
        id: pocketEdit

        Rectangle {
            color: (d.changeMatrix[styleData.row] && d.changeMatrix[styleData.row][styleData.column]) ? root.modifiedColor : "transparent"

            TextInput {
                anchors.fill: parent
                text: styleData.value
                color: styleData.textColor
                validator: IntValidator { bottom: d.minimumPocketId; top: d.maximumPocketId }

                onEditingFinished: d.updateToolData(styleData.row, styleData.column, Number(text));
                onActiveFocusChanged: if (activeFocus) { d.selectRow(styleData.row); }
            }
        }
    }

    Component {
        id: offsetEdit

        Rectangle {
            color: (d.changeMatrix[styleData.row] && d.changeMatrix[styleData.row][styleData.column]) ? root.modifiedColor : "transparent"

            TextInput {
                anchors.fill: parent
                readonly property int offset: 2
                readonly property int axis: styleData.column - offset
                text: styleData.value[axis]
                color: styleData.textColor
                validator: DoubleValidator { }

                onEditingFinished: d.updateAxisToolData(styleData.row, styleData.column, Number(text), axis)
                onActiveFocusChanged: if (activeFocus) { d.selectRow(styleData.row); }
            }
        }
    }

    Component {
        id: doubleEdit

        Rectangle {
            color: (d.changeMatrix[styleData.row] && d.changeMatrix[styleData.row][styleData.column]) ? root.modifiedColor : "transparent"

            TextInput {
                anchors.fill: parent
                text: styleData.value
                color: styleData.textColor
                validator: DoubleValidator { bottom: 0.0 }

                onEditingFinished: d.updateToolData(styleData.row, styleData.column, Number(text))
                onActiveFocusChanged: if (activeFocus) { d.selectRow(styleData.row); }
            }
        }
    }

    Component {
        id: orientationEdit

        Rectangle {
            color: (d.changeMatrix[styleData.row] && d.changeMatrix[styleData.row][styleData.column]) ? root.modifiedColor : "transparent"

            TextInput {
                anchors.fill: parent
                text: styleData.value
                color: styleData.textColor
                validator: IntValidator { bottom: d.minimumPosition; top: d.maximumPosition }

                onEditingFinished: d.updateToolData(styleData.row, styleData.column, Number(text))
                onActiveFocusChanged: if (activeFocus) { d.selectRow(styleData.row); }
            }
        }
    }

    Component {
        id: textEdit
        Rectangle {
            color: (d.changeMatrix[styleData.row] && d.changeMatrix[styleData.row][styleData.column]) ? root.modifiedColor : "transparent"

            TextInput {
                anchors.fill: parent
                text: styleData.value
                color: styleData.textColor

                onEditingFinished: d.updateToolData(styleData.row, styleData.column, text)
                onActiveFocusChanged: if (activeFocus) { d.selectRow(styleData.row); }
            }
        }
    }
}
