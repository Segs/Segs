/*
 * SEGS - Super Entity Game Server
 * http://www.segs.dev/
 * Copyright (c) 2006 - 2018 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Windows.Forms;
using Microsoft.Win32;
using System.Diagnostics;
using System.IO;

namespace RegistryWriter
{
    public partial class btnDialog : Form
    {
        RegistryKey rk;
        ProcessStartInfo startInfo = new ProcessStartInfo();
        string registryName;
        string keyName = "Auth";

        public btnDialog()
        {
            InitializeComponent();
        }

        private void btnLaunch_Click(object sender, EventArgs e)
        {
            registryName = tbValue.Text;
            ipToSpecialName (registryName);
            startProcess (txtDirectory.Text);
        }

        public string ipToSpecialName(string nameIP)
        {
            nameIP = nameIP.Replace(".", "_");
            nameIP = "IP" + nameIP;
            registryName = nameIP;
            return registryName;
        }

        public bool Write(string name, string keyName, object Value)
        {
            try
            {
                rk = Microsoft.Win32.Registry.CurrentUser.OpenSubKey("Software\\Cryptic");
                rk = Microsoft.Win32.Registry.CurrentUser.CreateSubKey ("Software\\Cryptic\\" + name);
                rk.SetValue (keyName.ToUpper (), Value);
                return true;
            }
            catch(Exception e)
            {
                Console.WriteLine (e.StackTrace);
                return false;
            }
        }

        private void btnDialogs_Click(object sender, EventArgs e)
        {
            DialogResult result = folderBrowserDialog1.ShowDialog();
            if(result == DialogResult.OK)
            {
                txtDirectory.Text = folderBrowserDialog1.SelectedPath;
            }
            Console.WriteLine(result);
        }

        public void startProcess(string directory)
        {
            startInfo.WorkingDirectory = directory;
            startInfo.Arguments = "-project " + registryName;
            string exename = FindCoHExec (directory);
            if(exename.Length == 0)
            {
                MessageBox.Show ("City of Heroes not found",
                                 "Important Note",
                                 MessageBoxButtons.OK,
                                 MessageBoxIcon.Exclamation,
                                 MessageBoxDefaultButton.Button1);
            }
            else
            {
                Write(registryName, keyName, tbValue.Text);
                startInfo.FileName = exename;
                Process.Start(startInfo);
            }
        }

        private string FindCoHExec(string directory)
        {
            string File1 = "CityOfHeroes.exe";
            string File2 = "CoX.exe";
            Directory.SetCurrentDirectory(directory);
            if(File.Exists(Path.Combine(directory, File1)))
            {
                return File1;
            }
            else if(File.Exists(Path.Combine(directory, File2)))
            {
                return File2;
            }
            return "";
        }

        private void txtDirectory_TextChanged(object sender, EventArgs e)
        {

        }

        private void tbValue_TextChanged(object sender, EventArgs e)
        {

        }

        private void btnDialog_Load(object sender, EventArgs e)
        {

        }
    }
}
