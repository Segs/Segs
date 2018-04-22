# Import required libraries and files
from PyQt5 import QtGui, QtCore, QtWidgets
import subprocess
import sys
import segsAdmin
import addnewuser
import dialog
import os
import platform
import images

# Variables
cmdwin = "cmd"
cmdlinux = "xterm"
cmdmac = "m"
opsys = platform.system()
dbfile_segs = "segs"
dbfile_segs_game = "segs_game"

print("***********************")
print("Welcome to SEGSAdmin")
print("***********************")


class SegsAdmin(QtWidgets.QMainWindow, QtWidgets.QApplication, segsAdmin.Ui_SEGSAdminTool):
    def __init__(self, parent=None):
        super(SegsAdmin, self).__init__(parent)
        self.setupUi(self)
        self.pleasewait.hide()
        self.runDBTool.clicked.connect(self.launchdbtool)
        self.form = SegsAddNewUser(self)
        self.dialog = SegsDialog(self)
        self.dialogconfirm = SegsConfirmDialog(self)
        self.createUser.clicked.connect(self.on_adduser_click)
        self.form.buttonBox.button(QtWidgets.QDialogButtonBox.Save).clicked.connect(self.createuser)
        self.dialog.buttonBox.button(QtWidgets.QDialogButtonBox.Ok).clicked.connect(self.dbtool_overwritedb)

    def on_adduser_click(self):
        self.form.usernameedit.clear()
        self.form.passedit.clear()
        self.form.accleveledit.clear()
        self.form.show()

    def outputtext(self, output):  # Not working... work in progress
        self.output.textCursor()
        self.output.insertPlainText("hello")
        self.output.insertPlainText(output)
        self.update()

    # Function for launching DB tool
    # TODO: Add some error handling from DBTool.exe output in case of DB creation error
    # TODO: Doesn't currently cater for only 'segs' or only 'segs_game' DB exists

    def launchdbtool(self):
        # Check if DB's already exist, if they do warn user then -f to overwrite

        if os.path.isfile(dbfile_segs) and \
                os.path.isfile(dbfile_segs_game):

            self.dialog.label.setText("Are you sure?")
            self.dialog.label_2.setText("We have detected existing databases, do you want to overwrite?")
            self.dialog.show()

        # DB's don't exist, create new
        else:
            self.dbtool_nooverwrite()

    def dbtool_overwritedb(self):  # TODO: Create backups of segs dbs in case of accidental overwrite
        if opsys == 'Windows':
            p = subprocess.Popen([cmdwin, '/c', 'dbtool create -f'], stdin=subprocess.PIPE, stdout=subprocess.PIPE)
            p.communicate(b"\n")

            self.dialogconfirm.label.setText("Complete")
            self.dialogconfirm.label_2.setText('')
            self.dialogconfirm.buttonBox.button(QtWidgets.QDialogButtonBox.Cancel).hide()
            self.dialogconfirm.show()

        if opsys == 'Linux':
            p = subprocess.Popen([cmdlinux, '-e', './dbtool create -f'], stdin=subprocess.PIPE, stdout=subprocess.PIPE)
            p.communicate(b"\n")

    @staticmethod
    def dbtool_nooverwrite():
        if opsys == 'Windows':
            p = subprocess.Popen([cmdwin, '/c', 'dbtool create'], stdin=subprocess.PIPE, stdout=subprocess.PIPE)
            p.communicate(b"\n")

        if opsys == 'Linux':
            p = subprocess.Popen([cmdlinux, '-e', './dbtool create'], stdin=subprocess.PIPE, stdout=subprocess.PIPE)
            p.communicate(b"\n")

    def createuser(self):  # TODO: Need to add some UI validation on blank fields
        username = self.form.usernameedit.text()
        password = self.form.passedit.text()
        acclevel = self.form.accleveledit.text()
        commandslinux = "./dbtool adduser -l" + " " + username + " " + "-p" + " " + password + " " + "-a" + " " + \
                        acclevel
        commandswin = "dbtool adduser -l" + " " + username + " " + "-p" + " " + password + " " + "-a" + " " + acclevel

        # if not (self.form.usernameedit == '') and (self.form.passedit == '') \
        # and (self.form.accleveledit == ''):

        if opsys == 'Windows':
                p = subprocess.Popen([cmdwin, '/c', commandswin], stdin=subprocess.PIPE, stdout=subprocess.PIPE)
                p.communicate(b"\n")

        if opsys == 'Linux':
                p = subprocess.Popen([cmdlinux, '-e', commandslinux], stdin=subprocess.PIPE, stdout=subprocess.PIPE)
                p.communicate(b"\n")

        # else:

            # self.dialog.label.setText("Please complete fields")
            # self.dialog.label_2.setText("Username, Password and Access Level must be completed")
            # self.dialog.buttonBox.button(QtWidgets.QDialogButtonBox.Cancel).hide()
            # self.dialog.show()


class SegsDialog(QtWidgets.QDialog, dialog.Ui_Dialog):
    def __init__(self, parent=None):
        super(SegsDialog, self).__init__(parent)
        self.setupUi(self)


class SegsConfirmDialog(QtWidgets.QDialog, dialog.Ui_Dialog):
    def __init__(self, parent=None):
        super(SegsConfirmDialog, self).__init__(parent)
        self.setupUi(self)


class SegsAddNewUser(QtWidgets.QDialog, addnewuser.Ui_adduserform):
    def __init__(self, parent=None):
        super(SegsAddNewUser, self).__init__(parent)
        self.setupUi(self)


# Opens application
def main():
    app = QtWidgets.QApplication(sys.argv)
    form = SegsAdmin()
    form.show()
    app.exec_()


if __name__ == '__main__':
    main()





