demo_font_flash为lvgl 使用外部flash存放字体demo



需要使用工具: [LvglFontTool_V0_4 ](https://forums.100ask.net/uploads/short-url/prUladIIuBh6gyIn0wr7hZEpe93.zip) 

1，使用LvglFontTool_V0_4工具选择类型外部bin字体生成字体与xxxx.c驱动文件

2，将bin文件写到flash中,可自己使用烧录工具下载,如果字体文件较小也可以使用demo_font_flash.c中if(0)下的烧录方式（具体看代码注释）但大部分情况字体文件都不会很小，最好使用烧录工具

3，参考xxxx.c驱动文件修改lvgl_flash_fonts.c中的字体信息

4，修改xmake.lua中编译外部flash font示例

**注意：如果开启SFUD_USING_FAST_READ确保flash直接焊接到板子上而不是杜邦线连接flash并且spi频率为51200000**

**注意：一定要用上面指定的LvglFontTool**

