using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace MOD_DDxeSR
{
    /**
     * 学会所有技艺
     */
    internal class AllArtistrySkillService
    {
        public static AllArtistrySkillService ME = new AllArtistrySkillService();

        public void onOpenSetting(Canvas ca)
        {
            ca.addOptionData(new CanvasOptionData("学习全部技艺", "HXC" + ModMain.nspace + "_allArtistrySkill", open));
        }


        /**
         * 启用
         */
        public void open()
        {
            //全技艺
            if (CommonDAO.ME.getData<int>(Constants.PALYER_ARTSKILL) == 0)
            {
                //技艺
                Dictionary<int, int> exists = new Dictionary<int, int>();
                var allArtSkill = g.data.world.allArtistrySkillID;
                foreach (var skill in allArtSkill)
                {
                    exists.Add(skill, 1);
                }
                //需要先删除堪舆
                int[] needDel = new int[] { 2011012, 2011013, 2011014, 2011015, 2011062, 2011063, 2011064, 2011065, 2011052, 2011053, 2011054, 2011055, 2011022, 2011023, 2011024, 2011025, 2011042, 2011043, 2011044, 2011045, 2011032, 2011033, 2011034, 2011035, 2011072, 2011073, 2011074, 2011075 };
                foreach (int id in needDel)
                {
                    if (exists.ContainsKey(id))
                    {
                        allArtSkill.Remove(id);
                    }
                }
                int[] allSkills = new int[] { 2011016 , 2011066 , 2011056, 2011026, 2011046, 2011036, 2011076,//堪舆
                2021012,2021022,2021032,2021042,2021052,2021062,2021072,2021082,2021092,2021102,2021103,2021113,2021163,2021173,2021183,2021193,2021203,2021212,2021224,2021234,2021244,2021254,2021264,2021274,2021284,2021294,2021304,2021314,2021324,2021334,2021344,2021354,2021143,2021153,2021355,2021365,2021375,2021385,2021395,2021405,2021415,2021425,2021435,2021445,2021455,2021465,2021475,2021485,2021495,2021505,2021123,2021133,2021506,2021516,2021526,2021536,2021546,2021556,2021566,2021576,2021586,2021596,2021606,2021616,2021626,2021636,2021646,2021656,2021666,2021676,2021686,2021696,//药谱
                6171102,6171103,6171104,6171105,6171202,6171203,6171204,6171205,6171302,6171303,6171304,6171305,6171402,6171403,6171404,6171405//鉴识
                };
                foreach (int id in allSkills)
                {
                    if (!exists.ContainsKey(id))
        