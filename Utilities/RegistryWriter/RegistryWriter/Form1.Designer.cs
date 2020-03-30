/*
 * SEGS - Super Entity Game Server
 * http://www.segs.dev/
 * Copyright (c) 2006 - 2018 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

namespace RegistryWriter
{
    partial class btnDialog
    {
        /// <summary>
        /// Required designer variable.
        /// </summary>
        private System.ComponentModel.IContainer components = null;

        /// <summary>
        /// Clean up any resources being used.
        /// </summary>
        /// <param name="disposing">true if managed resources should be disposed; otherwise, false.</param>
        protected override void Dispose(bool disposing)
        {
            if (disposing && (components != null))
            {
                components.Dispose();
            }
            base.Dispose(disposing);
        }

        #region Windows Form Designer generated code

        /// <summary>
        /// Required method for Designer support - do not modify
        /// the contents of this method with the code editor.
        /// </summary>
        private void InitializeComponent()
        {
            this.lblValue = new System.Windows.Forms.Label();
            this.btnLaunch = new System.Windows.Forms.Button();
            this.tbValue = new System.Windows.Forms.TextBox();
            this.lblKey = new System.Windows.Forms.Label();
            this.btnDialogs = new System.Windows.Forms.Button();
            this.txtDirectory = new System.Windows.Forms.TextBox();
            this.folderBrowserDialog1 = new System.Windows.Forms.FolderBrowserDialog();
            this.SuspendLayout();
            // 
            // lblValue
            // 
            this.lblValue.AutoSize = true;
            this.lblValue.Location = new System.Drawing.Point(52, 75);
            this.lblValue.Name = "lblValue";
            this.lblValue.Size = new System.Drawing.Size(70, 17);
            this.lblValue.TabIndex = 0;
            this.lblValue.Text = "Server IP:";
            // 
            // btnLaunch
            // 
            this.btnLaunch.Location = new System.Drawing.Point(128, 113);
            this.btnLaunch.Name = "btnLaunch";
            this.btnLaunch.Size = new System.Drawing.Size(106, 42);
            this.btnLaunch.TabIndex = 1;
            this.btnLaunch.Text = "Launch";
            this.btnLaunch.UseVisualStyleBackColor = true;
            this.btnLaunch.Click += new System.EventHandler(this.btnLaunch_Click);
            // 
            // tbValue
            // 
            this.tbValue.Location = new System.Drawing.Point(128, 75);
            this.tbValue.Name = "tbValue";
            this.tbValue.Size = new System.Drawing.Size(144, 22);
            this.tbValue.TabIndex = 2;
            this.tbValue.Text = "127.0.0.1";
            this.tbValue.TextChanged += new System.EventHandler(this.tbValue_TextChanged);
            // 
            // lblKey
            // 
            this.lblKey.AutoSize = true;
            this.lblKey.Location = new System.Drawing.Point(14, 37);
            this.lblKey.Name = "lblKey";
            this.lblKey.Size = new System.Drawing.Size(108, 17);
            this.lblKey.TabIndex = 3;
            this.lblKey.Text = "Client Directory:";
            // 
            // btnDialogs
            // 
            this.btnDialogs.Location = new System.Drawing.Point(278, 34);
            this.btnDialogs.Name = "btnDialogs";
            this.btnDialogs.Size = new System.Drawing.Size(51, 29);
            this.btnDialogs.TabIndex = 4;
            this.btnDialogs.Text = "...";
            this.btnDialogs.UseVisualStyleBackColor = true;
            this.btnDialogs.Click += new System.EventHandler(this.btnDialogs_Click);
            // 
            // txtDirectory
            // 
            this.txtDirectory.Location = new System.Drawing.Point(128, 37);
            this.txtDirectory.Name = "txtDirectory";
            this.txtDirectory.Size = new System.Drawing.Size(144, 22);
            this.txtDirectory.TabIndex = 5;
            this.txtDirectory.TextChanged += new System.EventHandler(this.txtDirectory_TextChanged);
            // 
            // btnDialog
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(8F, 16F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(354, 180);
            this.Controls.Add(this.txtDirectory);
            this.Controls.Add(this.btnDialogs);
            this.Controls.Add(this.lblKey);
            this.Controls.Add(this.tbValue);
            this.Controls.Add(this.btnLaunch);
            this.Controls.Add(this.lblValue);
            this.Name = "btnDialog";
            this.Text = "Segs Executor";
            this.Load += new System.EventHandler(this.btnDialog_Load);
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.Label lblValue;
        private System.Windows.Forms.Button btnLaunch;
        private System.Windows.Forms.TextBox tbValue;
        private System.Windows.Forms.Label lblKey;
        private System.Windows.Forms.Button btnDialogs;
        private System.Windows.Forms.TextBox txtDirectory;
        private System.Windows.Forms.FolderBrowserDialog folderBrowserDialog1;
    }
}
