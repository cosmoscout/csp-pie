/* global IApi,tasty */
class PieMenuApi extends IApi {
  name = 'pie';

  /**
   * @type {tasty.menu}
   */
  _menu;

  _parser;

  _state;

  /**
   *
   * @param {string} config
   * @param {string} container
   */
  initMenu(config, container = 'body') {
    this._menu = new tasty.menu(container, JSON.parse(config));
    this._menu.init();

    this._parser = new tasty.parser();
  }

  /**
   *
   * @param {string} structure
   */
  setStructure(structure) {
    structure = JSON.parse(structure);

    this._menu.setStructure(this._parser.parse(structure));
    this._menuEventListener();
  }

  _menuEventListener() {
    this._menu.selection$.subscribe(s => {
      console.log(s);
      if (s.type === 'itemSelection') {
        this._deactivate();
        if (typeof s.data !== "undefined" && typeof s.data.selected !== "undefined") {
          CosmoScout.callNative(s.target.itemId, s.data.selected);
        } else {

        }
      }
    });
  }

  enabled(state, posX, posY) {
    const pos = {
      x: posX,
      y: posY,
    };
    if (state === 1) {
      this._activate(pos);
    } else {
      this._deactivate();
    }
  }

  _deactivate() {
    this._menu.canvas.classList.remove('active');
    this._menu.canvas.style['background-color'] = 'rgba(0, 0, 0, 0)';
    this._state = 0;
/*    this._menu.inputDeactivation$.next({
      button: 0,
      buttons: 1,
      x: 0,
      y: 0,
    });*/
    this._menu.hide();
  }

  _activate(position) {
    console.log(position);
    this._menu.canvas.classList.add('active');
    this._menu.canvas.style['background-color'] = 'rgba(0, 0, 0, 0.5)';
    this._state = 1;
    this._menu.display(position);
/*    this._menu.inputActivation$.next({
      button: 0,
      buttons: 1,
      x: position.x,
      y: position.y,
    });*/
  }
}

(() => {
  CosmoScout.init(PieMenuApi);
})();