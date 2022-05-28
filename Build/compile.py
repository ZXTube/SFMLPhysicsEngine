import os

files = ""
for file in os.listdir():
    if file.endswith('.cpp'):
        OFile = file.strip('.cpp') + ".o"

        os.system(f"g++ -c {file} -I\"C:/Users/Proby Square 2/Desktop/VSCoding/C++/SFML/include\" -o\"Build/{OFile}\" -DSFML_STATIC")
        if file != "Main.cpp":
            files += " Build/" + OFile

os.system(f"g++ Build/main.o -o Build/main.exe -L\"C:/Users/Proby Square 2/Desktop/VSCoding/C++/SFML/lib\" {files} -lsfml-graphics-s -lsfml-window-s -lsfml-system-s -lopengl32 -lfreetype -lwinmm -lgdi32 -mwindows && Build\\main.exe")
