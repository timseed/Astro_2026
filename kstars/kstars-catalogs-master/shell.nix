{ pkgs ? import <nixos> {}
}:
let
  sphinx-click = pkgs.python3.pkgs.buildPythonPackage rec {
    pname = "sphinx-click";
    version = "3.0.0";

    src = pkgs.python3.pkgs.fetchPypi {
      inherit pname version;
      sha256 = "17hgqwwjbd8xndfyfr03lf7hsb9zxf0h4wi2kf3amqgajali03az";
    };

    propagatedBuildInputs = with pkgs.python3Packages; [ docutils sphinx
                                                         click pbr ];
    doCheck = false;
  };

  sphinx-reload = pkgs.python3.pkgs.buildPythonPackage rec {
    pname = "sphinx-reload";
    version = "0.2.0";

    src = pkgs.python3.pkgs.fetchPypi {
      inherit pname version;
      sha256 = "01zs2bvwmfvqx4ld05m5qi1h67dscpslxhgrwm70xpm6zszhf3l0";
    };

    propagatedBuildInputs = with pkgs.python3Packages; [ livereload ];
    doCheck = false;
  };

  my-python-packages = python-packages: with python-packages; [
    pybind11
    flake8
    jedi
    virtualenv
    mypy
    pip
    click
    astropy
    pandas
    wget
    #astroquery
    setuptools
    pydot
    sphinx
    #sphinx-book-theme
    sphinx-click
    sphinx-reload
    lxml
    xlrd
  ];

  python-with-my-packages = pkgs.python3.withPackages my-python-packages;
in pkgs.mkShell rec {
  name = "dev-shell";
  nativeBuildInputs = with pkgs; [ extra-cmake-modules ];
  inputsFrom = with pkgs; [ kstars ];
  buildInputs = with pkgs; [ pyright gsl blas ccache ninja qt6.full kstars python-with-my-packages fish sqlite sqlitebrowser black qtcreator qt6.wrapQtAppsHook breeze-icons];
  QT_PLUGIN_PATH = with pkgs.qt5; "${qtbase}/${qtbase.qtPluginPrefix}/";
  hardeningDisable = [ "all" ];
  CPATH = pkgs.lib.makeSearchPathOutput "dev" "include" buildInputs;
  CXXPATH = pkgs.lib.makeSearchPathOutput "dev" "include" buildInputs;
  shellHook = ''
        export MYPYPATH="$PWD/stubs";
        export PYTHONPATH="$PYTHONPATH:$PWD/kstars/kstars/python/build/lib.linux-x86_64-cpython-310/"
    '';
}
