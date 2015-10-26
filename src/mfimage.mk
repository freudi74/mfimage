##
## Auto Generated makefile by CodeLite IDE
## any manual changes will be erased      
##
## Debug
ProjectName            :=mfimage
ConfigurationName      :=Debug
WorkspacePath          := "/home/marco/dev/build/mfimage/projects/codelite"
ProjectPath            := "/home/marco/dev/build/mfimage/src"
IntermediateDirectory  :=./Debug
OutDir                 := $(IntermediateDirectory)
CurrentFileName        :=
CurrentFilePath        :=
CurrentFileFullPath    :=
User                   :=Marco Freudenberger
Date                   :=10/26/15
CodeLitePath           :="/home/marco/.codelite"
LinkerName             :=/usr/bin/g++
SharedObjectLinkerName :=/usr/bin/g++ -shared -fPIC
ObjectSuffix           :=.o
DependSuffix           :=.o.d
PreprocessSuffix       :=.i
DebugSwitch            :=-g 
IncludeSwitch          :=-I
LibrarySwitch          :=-l
OutputSwitch           :=-o 
LibraryPathSwitch      :=-L
PreprocessorSwitch     :=-D
SourceSwitch           :=-c 
OutputFile             :=$(IntermediateDirectory)/lib$(ProjectName).a
Preprocessors          :=
ObjectSwitch           :=-o 
ArchiveOutputSwitch    := 
PreprocessOnlySwitch   :=-E
ObjectsFileList        :="mfimage.txt"
PCHCompileFlags        :=
MakeDirCommand         :=mkdir -p
LinkOptions            :=  $(pkg-config --libs libjpeg libtiff-4 libpng libopenjp2)
IncludePath            :=  $(IncludeSwitch). $(IncludeSwitch). 
IncludePCH             := 
RcIncludePath          := 
Libs                   := 
ArLibs                 :=  
LibPath                := $(LibraryPathSwitch). 

##
## Common variables
## AR, CXX, CC, AS, CXXFLAGS and CFLAGS can be overriden using an environment variables
##
AR       := /usr/bin/ar rcu
CXX      := /usr/bin/g++
CC       := /usr/bin/gcc
CXXFLAGS :=  -g -std=c++14 -Wall  -DSVN_REVISION=\"\"  $(Preprocessors)
CFLAGS   :=  -g  -DSVN_REVISION=\"\"  $(Preprocessors)
ASFLAGS  := 
AS       := /usr/bin/as


##
## User defined environment variables
##
CodeLiteDir:=/usr/share/codelite
Objects0=$(IntermediateDirectory)/ColorManager.cpp$(ObjectSuffix) $(IntermediateDirectory)/Image.cpp$(ObjectSuffix) $(IntermediateDirectory)/ImageCoder.cpp$(ObjectSuffix) $(IntermediateDirectory)/ImageCoderBitmap.cpp$(ObjectSuffix) $(IntermediateDirectory)/ImageCoderJpeg.cpp$(ObjectSuffix) $(IntermediateDirectory)/ImageCoderPcx.cpp$(ObjectSuffix) $(IntermediateDirectory)/ImageCoderTiff.cpp$(ObjectSuffix) $(IntermediateDirectory)/TimeStamper.cpp$(ObjectSuffix) $(IntermediateDirectory)/stdafx.cpp$(ObjectSuffix) $(IntermediateDirectory)/ImageCoderGif.cpp$(ObjectSuffix) \
	$(IntermediateDirectory)/ImageCoderPng.cpp$(ObjectSuffix) $(IntermediateDirectory)/ImageCoderTarga.cpp$(ObjectSuffix) $(IntermediateDirectory)/PixelMode.cpp$(ObjectSuffix) $(IntermediateDirectory)/ImageCoderJpeg2000.cpp$(ObjectSuffix) $(IntermediateDirectory)/ImageCoderProperties.cpp$(ObjectSuffix) 



Objects=$(Objects0) 

##
## Main Build Targets 
##
.PHONY: all clean PreBuild PrePreBuild PostBuild
all: $(IntermediateDirectory) $(OutputFile)

$(OutputFile): $(Objects)
	@$(MakeDirCommand) $(@D)
	@echo "" > $(IntermediateDirectory)/.d
	@echo $(Objects0)  > $(ObjectsFileList)
	$(AR) $(ArchiveOutputSwitch)$(OutputFile) @$(ObjectsFileList) $(ArLibs)
	@$(MakeDirCommand) "/home/marco/dev/build/mfimage/projects/codelite/.build-debug"
	@echo rebuilt > "/home/marco/dev/build/mfimage/projects/codelite/.build-debug/mfimage"

./Debug:
	@test -d ./Debug || $(MakeDirCommand) ./Debug

PreBuild:


##
## Objects
##
$(IntermediateDirectory)/ColorManager.cpp$(ObjectSuffix): ColorManager.cpp $(IntermediateDirectory)/ColorManager.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/marco/dev/build/mfimage/src/ColorManager.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/ColorManager.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/ColorManager.cpp$(DependSuffix): ColorManager.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/ColorManager.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/ColorManager.cpp$(DependSuffix) -MM "ColorManager.cpp"

$(IntermediateDirectory)/ColorManager.cpp$(PreprocessSuffix): ColorManager.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/ColorManager.cpp$(PreprocessSuffix) "ColorManager.cpp"

$(IntermediateDirectory)/Image.cpp$(ObjectSuffix): Image.cpp $(IntermediateDirectory)/Image.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/marco/dev/build/mfimage/src/Image.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/Image.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/Image.cpp$(DependSuffix): Image.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/Image.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/Image.cpp$(DependSuffix) -MM "Image.cpp"

$(IntermediateDirectory)/Image.cpp$(PreprocessSuffix): Image.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/Image.cpp$(PreprocessSuffix) "Image.cpp"

$(IntermediateDirectory)/ImageCoder.cpp$(ObjectSuffix): ImageCoder.cpp $(IntermediateDirectory)/ImageCoder.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/marco/dev/build/mfimage/src/ImageCoder.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/ImageCoder.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/ImageCoder.cpp$(DependSuffix): ImageCoder.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/ImageCoder.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/ImageCoder.cpp$(DependSuffix) -MM "ImageCoder.cpp"

$(IntermediateDirectory)/ImageCoder.cpp$(PreprocessSuffix): ImageCoder.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/ImageCoder.cpp$(PreprocessSuffix) "ImageCoder.cpp"

$(IntermediateDirectory)/ImageCoderBitmap.cpp$(ObjectSuffix): ImageCoderBitmap.cpp $(IntermediateDirectory)/ImageCoderBitmap.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/marco/dev/build/mfimage/src/ImageCoderBitmap.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/ImageCoderBitmap.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/ImageCoderBitmap.cpp$(DependSuffix): ImageCoderBitmap.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/ImageCoderBitmap.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/ImageCoderBitmap.cpp$(DependSuffix) -MM "ImageCoderBitmap.cpp"

$(IntermediateDirectory)/ImageCoderBitmap.cpp$(PreprocessSuffix): ImageCoderBitmap.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/ImageCoderBitmap.cpp$(PreprocessSuffix) "ImageCoderBitmap.cpp"

$(IntermediateDirectory)/ImageCoderJpeg.cpp$(ObjectSuffix): ImageCoderJpeg.cpp $(IntermediateDirectory)/ImageCoderJpeg.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/marco/dev/build/mfimage/src/ImageCoderJpeg.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/ImageCoderJpeg.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/ImageCoderJpeg.cpp$(DependSuffix): ImageCoderJpeg.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/ImageCoderJpeg.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/ImageCoderJpeg.cpp$(DependSuffix) -MM "ImageCoderJpeg.cpp"

$(IntermediateDirectory)/ImageCoderJpeg.cpp$(PreprocessSuffix): ImageCoderJpeg.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/ImageCoderJpeg.cpp$(PreprocessSuffix) "ImageCoderJpeg.cpp"

$(IntermediateDirectory)/ImageCoderPcx.cpp$(ObjectSuffix): ImageCoderPcx.cpp $(IntermediateDirectory)/ImageCoderPcx.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/marco/dev/build/mfimage/src/ImageCoderPcx.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/ImageCoderPcx.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/ImageCoderPcx.cpp$(DependSuffix): ImageCoderPcx.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/ImageCoderPcx.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/ImageCoderPcx.cpp$(DependSuffix) -MM "ImageCoderPcx.cpp"

$(IntermediateDirectory)/ImageCoderPcx.cpp$(PreprocessSuffix): ImageCoderPcx.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/ImageCoderPcx.cpp$(PreprocessSuffix) "ImageCoderPcx.cpp"

$(IntermediateDirectory)/ImageCoderTiff.cpp$(ObjectSuffix): ImageCoderTiff.cpp $(IntermediateDirectory)/ImageCoderTiff.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/marco/dev/build/mfimage/src/ImageCoderTiff.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/ImageCoderTiff.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/ImageCoderTiff.cpp$(DependSuffix): ImageCoderTiff.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/ImageCoderTiff.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/ImageCoderTiff.cpp$(DependSuffix) -MM "ImageCoderTiff.cpp"

$(IntermediateDirectory)/ImageCoderTiff.cpp$(PreprocessSuffix): ImageCoderTiff.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/ImageCoderTiff.cpp$(PreprocessSuffix) "ImageCoderTiff.cpp"

$(IntermediateDirectory)/TimeStamper.cpp$(ObjectSuffix): TimeStamper.cpp $(IntermediateDirectory)/TimeStamper.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/marco/dev/build/mfimage/src/TimeStamper.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/TimeStamper.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/TimeStamper.cpp$(DependSuffix): TimeStamper.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/TimeStamper.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/TimeStamper.cpp$(DependSuffix) -MM "TimeStamper.cpp"

$(IntermediateDirectory)/TimeStamper.cpp$(PreprocessSuffix): TimeStamper.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/TimeStamper.cpp$(PreprocessSuffix) "TimeStamper.cpp"

$(IntermediateDirectory)/stdafx.cpp$(ObjectSuffix): stdafx.cpp $(IntermediateDirectory)/stdafx.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/marco/dev/build/mfimage/src/stdafx.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/stdafx.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/stdafx.cpp$(DependSuffix): stdafx.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/stdafx.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/stdafx.cpp$(DependSuffix) -MM "stdafx.cpp"

$(IntermediateDirectory)/stdafx.cpp$(PreprocessSuffix): stdafx.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/stdafx.cpp$(PreprocessSuffix) "stdafx.cpp"

$(IntermediateDirectory)/ImageCoderGif.cpp$(ObjectSuffix): ImageCoderGif.cpp $(IntermediateDirectory)/ImageCoderGif.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/marco/dev/build/mfimage/src/ImageCoderGif.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/ImageCoderGif.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/ImageCoderGif.cpp$(DependSuffix): ImageCoderGif.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/ImageCoderGif.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/ImageCoderGif.cpp$(DependSuffix) -MM "ImageCoderGif.cpp"

$(IntermediateDirectory)/ImageCoderGif.cpp$(PreprocessSuffix): ImageCoderGif.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/ImageCoderGif.cpp$(PreprocessSuffix) "ImageCoderGif.cpp"

$(IntermediateDirectory)/ImageCoderPng.cpp$(ObjectSuffix): ImageCoderPng.cpp $(IntermediateDirectory)/ImageCoderPng.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/marco/dev/build/mfimage/src/ImageCoderPng.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/ImageCoderPng.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/ImageCoderPng.cpp$(DependSuffix): ImageCoderPng.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/ImageCoderPng.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/ImageCoderPng.cpp$(DependSuffix) -MM "ImageCoderPng.cpp"

$(IntermediateDirectory)/ImageCoderPng.cpp$(PreprocessSuffix): ImageCoderPng.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/ImageCoderPng.cpp$(PreprocessSuffix) "ImageCoderPng.cpp"

$(IntermediateDirectory)/ImageCoderTarga.cpp$(ObjectSuffix): ImageCoderTarga.cpp $(IntermediateDirectory)/ImageCoderTarga.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/marco/dev/build/mfimage/src/ImageCoderTarga.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/ImageCoderTarga.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/ImageCoderTarga.cpp$(DependSuffix): ImageCoderTarga.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/ImageCoderTarga.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/ImageCoderTarga.cpp$(DependSuffix) -MM "ImageCoderTarga.cpp"

$(IntermediateDirectory)/ImageCoderTarga.cpp$(PreprocessSuffix): ImageCoderTarga.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/ImageCoderTarga.cpp$(PreprocessSuffix) "ImageCoderTarga.cpp"

$(IntermediateDirectory)/PixelMode.cpp$(ObjectSuffix): PixelMode.cpp $(IntermediateDirectory)/PixelMode.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/marco/dev/build/mfimage/src/PixelMode.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/PixelMode.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/PixelMode.cpp$(DependSuffix): PixelMode.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/PixelMode.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/PixelMode.cpp$(DependSuffix) -MM "PixelMode.cpp"

$(IntermediateDirectory)/PixelMode.cpp$(PreprocessSuffix): PixelMode.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/PixelMode.cpp$(PreprocessSuffix) "PixelMode.cpp"

$(IntermediateDirectory)/ImageCoderJpeg2000.cpp$(ObjectSuffix): ImageCoderJpeg2000.cpp $(IntermediateDirectory)/ImageCoderJpeg2000.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/marco/dev/build/mfimage/src/ImageCoderJpeg2000.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/ImageCoderJpeg2000.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/ImageCoderJpeg2000.cpp$(DependSuffix): ImageCoderJpeg2000.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/ImageCoderJpeg2000.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/ImageCoderJpeg2000.cpp$(DependSuffix) -MM "ImageCoderJpeg2000.cpp"

$(IntermediateDirectory)/ImageCoderJpeg2000.cpp$(PreprocessSuffix): ImageCoderJpeg2000.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/ImageCoderJpeg2000.cpp$(PreprocessSuffix) "ImageCoderJpeg2000.cpp"

$(IntermediateDirectory)/ImageCoderProperties.cpp$(ObjectSuffix): ImageCoderProperties.cpp $(IntermediateDirectory)/ImageCoderProperties.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/marco/dev/build/mfimage/src/ImageCoderProperties.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/ImageCoderProperties.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/ImageCoderProperties.cpp$(DependSuffix): ImageCoderProperties.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/ImageCoderProperties.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/ImageCoderProperties.cpp$(DependSuffix) -MM "ImageCoderProperties.cpp"

$(IntermediateDirectory)/ImageCoderProperties.cpp$(PreprocessSuffix): ImageCoderProperties.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/ImageCoderProperties.cpp$(PreprocessSuffix) "ImageCoderProperties.cpp"


-include $(IntermediateDirectory)/*$(DependSuffix)
##
## Clean
##
clean:
	$(RM) -r ./Debug/


