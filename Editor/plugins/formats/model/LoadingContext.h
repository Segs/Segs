using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Runtime.Serialization.Formatters.Binary;
using System.Text.RegularExpressions;
using SEGSRuntime;
using UnityEngine;

namespace SEGSRuntime
{
    internal class LoadingContext
    {
        public int last_node_id = 0; // used to create new number suffixes for generic nodes
        public string m_base_path;
        public NameList m_renamer=new NameList(); // used to rename prefab nodes to keep all names unique in the loaded graph
        public SceneGraph m_target;
        public int nesting_level;


        public LoadingContext(int nestingLevel)
        {
            nesting_level = nestingLevel;
        }

        internal string buildBaseName(string path)
        {
            List<string> z = path.Split('/').ToList();

            if (z.Count > 1)
                z.RemoveAt(z.Count - 1); // remove file name

            if (z[0] == "object_library")
                z.RemoveAt(0);
            else if (z.Contains("maps", StringComparer.CurrentCultureIgnoreCase))
            {
                while (0 != String.Compare("maps", z[0], StringComparison.OrdinalIgnoreCase))
                    z.RemoveAt(0);
            }

            return String.Join("/", z);
        }

        public bool loadSceneGraph(string path, PrefabStore prefabs)
        {
            string binName = mapNameToPath(path);
            SceneGraph_Data serialized_graph = new SceneGraph_Data();
            m_renamer.basename = buildBaseName(path);
            binName = binName.Replace("Chunks.bin", "CHUNKS.bin");
            if (!serialized_graph.LoadSceneData(binName))
                return false;

            serialized_graph.serializeIn(this, prefabs, binName);
            return true;
        }

        internal string mapNameToPath(string name)
        {
            int start_idx = name.IndexOf("object_library", StringComparison.OrdinalIgnoreCase);
            if (-1 == start_idx)
                start_idx = name.IndexOf("maps", StringComparison.OrdinalIgnoreCase);

            string buf = m_base_path + "geobin/" + name.Substring(start_idx);

            int last_dot = buf.LastIndexOf('.');
            if (-1 == last_dot)
                buf += ".bin";
            else if (!buf.Contains(".crl"))
            {
                buf = buf.Remove(last_dot, buf.Length - last_dot);
                buf = buf.Insert(last_dot, ".bin");
            }

            return buf;
        }

        public string groupRename(string oldname, bool is_def)
        {
            string str = oldname.Contains('/') ? String.Copy(oldname) : m_renamer.basename + '/' + oldname;
            if (Tools.groupInLibSub(str))
                return str;
            if (!is_def && !str.ToLower().Contains("/grp") && !str.ToLower().Contains("/map"))
                return str;

            string querystring = str.ToLower();
            string res = null;
            if (m_renamer.new_names.TryGetValue(querystring, out res))
                return res;

            string prefix = str;
            int gidx = prefix.IndexOf("/grp", 0, StringComparison.OrdinalIgnoreCase);
            if (gidx != -1)
            {
                prefix = prefix.Substring(gidx + 4); // skip /grp
                var m = Regex.Match(prefix, "[^A-Za-z]");
                int finidx = prefix.Length;
                if (m.Success)
                    finidx = m.Index;
                prefix = prefix.Substring(0, finidx); // collect chars to first non-letter
            }
            else
            {
                if (prefix.ToLower().Contains("/map"))
                    prefix = "maps/grp";
                else
                {
                    Debug.Log("bad def name:" + prefix);
                    prefix = "baddef";
                }
            }

            string tgt = groupMakeName(prefix);
            m_renamer.new_names[querystring] = tgt;
            return tgt;
        }

        private string groupMakeName(string basep)
        {
            string buf;
            do
                buf = basep + (++last_node_id).ToString();
            while (m_target.getNodeByName(buf) != null);

            return buf;
        }

        public void loadSubgraph(string geopath, PrefabStore prefabs)
        {
            var fi = new FileInfo(geopath);
            LoadingContext tmp = new LoadingContext(nesting_level + 1);
            tmp.m_renamer = DeepClone<NameList>(m_renamer);
            tmp.m_target = m_target;
            tmp.last_node_id = last_node_id;
            tmp.m_base_path = m_base_path;
            string complete_base_name = Path.GetFileNameWithoutExtension(fi.Name);
            
            tmp.loadSceneGraph(fi.DirectoryName + "/" + complete_base_name + ".txt", prefabs);
        }
        public static T DeepClone<T>(T obj)
        {
            using (var ms = new MemoryStream())
            {
                var formatter = new BinaryFormatter();
                formatter.Serialize(ms, obj);
                ms.Position = 0;

                return (T) formatter.Deserialize(ms);
            }
        }
    }
}