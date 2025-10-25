# escape=`

FROM mcr.microsoft.com/windows/servercore:ltsc2022

SHELL ["powershell","-NoLogo","-ExecutionPolicy","Bypass","-Command"]

# 1) Instalar Visual Studio Build Tools (C++ toolchain + Windows SDK)
#    Componentes principais:
#    - Microsoft.VisualStudio.Workload.VCTools: toolchain C/C++
#    - Microsoft.VisualStudio.Component.VC.Tools.x86.x64: cl/link 32/64-bit
#    - Windows 10 SDK 19041 (suficiente p/ maioria dos casos)
ADD https://aka.ms/vs/17/release/vs_BuildTools.exe C:\\temp\\vs_buildtools.exe

RUN Start-Process -FilePath C:\\temp\\vs_buildtools.exe -ArgumentList `
      '--quiet --wait --norestart --nocache `
      --installPath C:\\BuildTools `
      --add Microsoft.VisualStudio.Workload.VCTools `
      --add Microsoft.VisualStudio.Component.VC.Tools.x86.x64 `
      --add Microsoft.VisualStudio.Component.Windows10SDK.19041' -Wait; `
    Remove-Item C:\\temp\\vs_buildtools.exe -Force

# 2) (Opcional, mas útil) instalar CMake e Ninja via Chocolatey
RUN [System.Net.ServicePointManager]::SecurityProtocol = [System.Net.SecurityProtocolType]::Tls12; `
    Set-ExecutionPolicy Bypass -Scope Process -Force; `
    iwr https://community.chocolatey.org/install.ps1 -UseBasicParsing | iex; `
    choco install -y cmake --install-arguments '"ADD_CMAKE_TO_PATH=System"'; `
    choco install -y ninja git

# Variáveis padrão (build 64-bit)
ENV VSCMD_ARG_TGT_ARCH=x64
ENV VSCMD_ARG_HOST_ARCH=x64

# Copiar script de build
WORKDIR C:\\work
COPY scripts\\build.ps1 C:\\work\\build.ps1

# ENTRYPOINT: habilita ambiente MSVC e roda o script
SHELL ["cmd", "/S", "/C"]
ENTRYPOINT "C:\\BuildTools\\Common7\\Tools\\VsDevCmd.bat && powershell -ExecutionPolicy Bypass -File C:\\work\\build.ps1"
