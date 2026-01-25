using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace MOD_DDxeSR
{
    /**
     * 戒指查看论道结果
     */
    internal class RingSeeChatService
    {
        public static RingSeeChatService ME = new RingSeeChatService();

        //-1关闭 0未配置 1开启
        private int openSetting = 0;

        private string code = "ringSeeChat";

        public void onIntoWorld()
        {
            openSetting = loadOpen();
            //开启
            if (getOpen())
            {
                open(false);
            }
            //不开启走默认
        }

        public void onOpenSetting(Canvas ca)
        {
            if (getOpen())
            {
                ca.addOptionData(new CanvasOptionData("关闭戒指查看论道结果", "HXC" + ModMain.nspace + "_" + code, close));
            }else
            {
                ca.addOptionData(new CanvasOptionData("开启戒指查看论道结果", "HXC" + ModMain.nspace + "_" + code, open));
            }
        }

        
        public void open()
        {
            open(true);
        }
        /**
         * 启用
         */
        public void open(bool alert = true)
        {
            //给所有戒指添加额外效果
            int effect1 = ModMain.mid + 10;//查看论道结果
            var allRings = g.conf.ringBase._allConfList;
            foreach (var ring in allRings)
            {
                ring.effectValue = Util.addCommand(ring.effectValue, effect1);
            }
            openSetting = 1;
            saveOpen(openSetting);
            if(alert){
                ConsUtil.alert("开启戒指查看论道结果，保存并重启存档生效");
            }
        }

        public void close()
        {
            openSetting = -1;
            saveOpen(openSetting);
            ConsUtil.alert("关闭戒指查看论道结果，保存并重启存档生效");
        }
      
        private void saveOpen(int open)
        {
            CommonDAO.ME.saveData(code + ".open", open);
        }
        private int loadOpen()
        {
            return CommonDAO.ME.getData<int>(code + ".open");
        }
        private