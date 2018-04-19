# -*- coding: utf-8 -*-

# Form implementation generated from reading ui file 'addnewuser.ui'
#
# Created by: PyQt5 UI code generator 5.10.1
#
# WARNING! All changes made in this file will be lost!

from PyQt5 import QtCore, QtGui, QtWidgets

class Ui_adduserform(object):
    def setupUi(self, adduserform):
        adduserform.setObjectName("adduserform")
        adduserform.resize(400, 159)
        self.buttonBox = QtWidgets.QDialogButtonBox(adduserform)
        self.buttonBox.setGeometry(QtCore.QRect(30, 120, 341, 32))
        self.buttonBox.setOrientation(QtCore.Qt.Horizontal)
        self.buttonBox.setStandardButtons(QtWidgets.QDialogButtonBox.Cancel|QtWidgets.QDialogButtonBox.Save)
        self.buttonBox.setObjectName("buttonBox")
        self.widget = QtWidgets.QWidget(adduserform)
        self.widget.setGeometry(QtCore.QRect(20, 20, 361, 91))
        self.widget.setObjectName("widget")
        self.formLayout = QtWidgets.QFormLayout(self.widget)
        self.formLayout.setContentsMargins(0, 0, 0, 0)
        self.formLayout.setObjectName("formLayout")
        self.username = QtWidgets.QLabel(self.widget)
        font = QtGui.QFont()
        font.setPointSize(12)
        self.username.setFont(font)
        self.username.setObjectName("username")
        self.formLayout.setWidget(0, QtWidgets.QFormLayout.LabelRole, self.username)
        self.usernameedit = QtWidgets.QLineEdit(self.widget)
        self.usernameedit.setObjectName("usernameedit")
        self.formLayout.setWidget(0, QtWidgets.QFormLayout.FieldRole, self.usernameedit)
        self.password = QtWidgets.QLabel(self.widget)
        font = QtGui.QFont()
        font.setPointSize(12)
        self.password.setFont(font)
        self.password.setObjectName("password")
        self.formLayout.setWidget(1, QtWidgets.QFormLayout.LabelRole, self.password)
        self.passedit = QtWidgets.QLineEdit(self.widget)
        self.passedit.setObjectName("passedit")
        self.formLayout.setWidget(1, QtWidgets.QFormLayout.FieldRole, self.passedit)
        self.acclevel = QtWidgets.QLabel(self.widget)
        font = QtGui.QFont()
        font.setPointSize(12)
        self.acclevel.setFont(font)
        self.acclevel.setObjectName("acclevel")
        self.formLayout.setWidget(2, QtWidgets.QFormLayout.LabelRole, self.acclevel)
        self.accleveledit = QtWidgets.QSpinBox(self.widget)
        self.accleveledit.setMinimum(1)
        self.accleveledit.setMaximum(9)
        self.accleveledit.setObjectName("accleveledit")
        self.formLayout.setWidget(2, QtWidgets.QFormLayout.FieldRole, self.accleveledit)

        self.retranslateUi(adduserform)
        self.buttonBox.accepted.connect(adduserform.accept)
        self.buttonBox.rejected.connect(adduserform.reject)
        QtCore.QMetaObject.connectSlotsByName(adduserform)

    def retranslateUi(self, adduserform):
        _translate = QtCore.QCoreApplication.translate
        adduserform.setWindowTitle(_translate("adduserform", "Add New User"))
        self.username.setText(_translate("adduserform", "Username:"))
        self.password.setText(_translate("adduserform", "Password:"))
        self.acclevel.setText(_translate("adduserform", "Access Level:"))

