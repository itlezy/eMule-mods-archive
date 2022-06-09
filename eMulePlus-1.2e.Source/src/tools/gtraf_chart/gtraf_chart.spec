a = Analysis(['C:\\Python22\\Installer\\support\\_mountzlib.py', 'C:\\Python22\\Installer\\support\\useUnicode.py', 'gtraf_chart.py'],
             pathex=[])
pyz = PYZ(a.pure)
exe = EXE(pyz,
          a.scripts,
          exclude_binaries=1,
          name='buildgtraf_chart/gtraf_chart.exe',
          debug=0,
          strip=0,
          console=0 )
coll = COLLECT( exe,
               a.binaries,
               strip=0,
               name='distgtraf_chart')
