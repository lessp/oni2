open Revery.UI;
open Revery.UI.Components;
open Oni_Model;
open Oni_Core;

let component = React.component("Home");

let containerStyles = Style.[flexGrow(1)];

let homeContainerStyles = (theme: Theme.t) =>
  Style.[
    backgroundColor(theme.colors.background),
    color(theme.colors.foreground),
    position(`Absolute),
    top(0),
    left(0),
    right(0),
    bottom(0),
    flexDirection(`Column),
    alignItems(`Center),
    justifyContent(`Center),
  ];

let startEditor = () => GlobalContext.current().dispatch(ShowEditor);
let homeMenuContainer = (theme: Theme.t) =>
  Style.[
    width(500),
    height(300),
    backgroundColor(theme.colors.statusBarBackground),
    flexDirection(`Column),
    justifyContent(`Center),
    alignItems(`Center),
  ];

let createElement = (~children as _, ~theme: Theme.t, ~state: State.t, ()) =>
  component(hooks => {
    let (animatedOpacity, hooks) =
      Hooks.animation(
        Animated.floatValue(0.),
        {
          toValue: 1.0,
          duration: Seconds(0.3),
          delay: Seconds(2.),
          repeat: false,
          easing: Animated.linear,
        },
        hooks,
      );
    let oniFontFamily = state.uiFont.fontFile;
    (
      hooks,
      <View style=containerStyles>
        <View
          style=Style.[
            opacity(animatedOpacity),
            ...homeContainerStyles(theme),
          ]>
          <Text
            text="Welcome to Oni2"
            style=Style.[fontFamily(oniFontFamily), fontSize(20)]
          />
          <Image
            src="logo.png"
            style=Style.[width(50), height(50), marginBottom(20)]
          />
          <View style={homeMenuContainer(theme)}>
            <Button
              title="Open Editor"
              fontFamily=oniFontFamily
              color={theme.colors.background}
              fontSize=20
              width=120
              height=40
              onClick=startEditor
            />
          </View>
        </View>
      </View>,
    );
  });