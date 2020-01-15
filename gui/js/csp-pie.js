/* global IApi,CosmoScout,tasty */

/**
 * Pie Menu API
 */
class PieMenuApi extends IApi {
  /**
   * @inheritDoc
   */
  name = 'pie';

  /**
   * @type {tasty.menu}
   */
  _menu;

  /**
   * @type {tasty.parser}
   */
  _parser;

  /**
   * ItemID -> MenuItem map from tasty.parser
   *
   * @type {Map<string, *>}
   */
  _map;

  /**
   * Initializes the menu into a set container and sets the config values
   *
   * @param {string} config Pie Menu config
   * @param {string} container Container CSS selector
   */
  initMenu(config, container = 'body') {
    this._menu = new tasty.menu(container, JSON.parse(config));
    this._menu.init();

    this._parser = new tasty.parser();
  }

  /**
   * Set the menu structure as JSON.
   * Gets parsed by tasty.parser
   *
   * @param {string} structure JSON Menu Structure
   */
  setStructure(structure) {
    structure = JSON.parse(structure);

    this._menu.setStructure(this._parser.parse(structure));
    this._map = this._parser.map;
    this._menuEventListener();
  }

  /**
   * Set a checkbox to selected
   *
   * @param {string} itemId
   * @param {boolean|number} selected
   */
  setCheckboxSelected(itemId, selected) {
    if (this._hasItemAndTypeMatches(itemId, 'checkbox')) {
      const item = this._map.get(itemId);

      // Check if checkbox is in a radio group
      // If so don't select
      if (typeof item.parent !== 'undefined' && typeof item.parent.TYPE !== 'undefined' && item.parent.TYPE === 'radiogroup') {
        console.warn('Use "selectRadioGroupItem" to select a checkbox in a RadioGroup instead of directly trying to access it.');
        return;
      }

      if (Boolean(Number(selected)) === true) {
        item.select();
      } else {
        item.deselect();
      }
    }
  }

  /**
   * Select an item of a radio group
   *
   * @param {string} groupId
   * @param {string} itemId
   */
  selectRadioGroupItem(groupId, itemId) {
    if (this._hasItemAndTypeMatches(groupId, 'radiogroup')) {
      this._map.get(groupId).select(itemId);
    }
  }

  /**
   * Set the value of a ribbonslider
   *
   * @param {string} itemId
   * @param {number} value
   */
  setSliderValue(itemId, value) {
    if (this._hasItemAndTypeMatches(itemId, 'ribbonslider')) {
      this._map.get(itemId).value = Number(value);
    }
  }

  /**
   * Shows / Hides the Pie Menu based on state
   *
   * @param {number} state 1 = Display / 0 = Hide
   * @param {number} posX X-Position to display the menu on
   * @param {number} posY Y-Position to display the menu on
   */
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

  _menuEventListener() {
    this._menu.selection$.subscribe((s) => {
      console.log(s);
      if (s.type === 'itemSelection') {
        this._deactivate();
        if (typeof s.data !== 'undefined' && typeof s.data.selected !== 'undefined') {
          CosmoScout.callNative('pie_item_toggled', s.target.itemId, s.data.selected);
        } else {
          CosmoScout.callNative('pie_item_action', s.target.itemId);
        }
      } if (s.type === 'sliderValueChanging') {
        CosmoScout.callNative('pie_slider_changed', s.target.itemId, s.data.value);
      }
    });
  }

  /**
   * Checks if the parser map contains a given item id and if the item matches the given type
   *
   * @param {string} itemId
   * @param {string} type checkbox|radiogroup|ribbonslider
   * @return {boolean} True if has item and type matches
   * @private
   */
  _hasItemAndTypeMatches(itemId, type) {
    return this._map.has(itemId) && this._map.get(itemId).TYPE === type.toLowerCase();
  }

  /**
   * Deactivates the menu
   *
   * @private
   */
  _deactivate() {
    this._menu.canvas.classList.remove('active');
    this._menu.canvas.style['background-color'] = 'rgba(0, 0, 0, 0)';
    this._menu.hide();
  }

  /**
   * Displays the menu on a given position
   *
   * @param {{x:number, y:number}} position
   * @private
   */
  _activate(position) {
    this._menu.canvas.classList.add('active');
    this._menu.canvas.style['background-color'] = 'rgba(0, 0, 0, 0.01)';
    this._menu.display(position);
  }
}

(() => {
  CosmoScout.init(PieMenuApi);
})();
