// append new board name here

export const JemaOSBoardNameTitleMap: {[index: string]: string} = {
  'itnt-72': 'itNT 72',
  'itnt-72-go': 'itNT 72 GO',
  'itnt-x': 'itNT X',
  'itnt-80-go': 'itNT 80 GO',
  'link_jemaos': 'JemaOS Link',
  'samus_jemaos': 'JemaOS Samus',
  'jerry-jemaos': 'JemaOS Jerry',
  'fizz-jemaos': 'Jemarbox',
  'magicbook': 'MagicBook',
  'gpdpocket2': 'GPD Pocket 2',
  'pinebook-pro': 'Pinebook Pro',
  'surface-pro3': 'Surface Pro 3',
  'surface-pro4': 'Surface Pro 4',
  'surface-pro5': 'Surface Pro 5',
  'surface-pro6': 'Surface Pro 6',
  'surface-pro7': 'Surface Pro 7',
  'surface-pro7p': 'Surface Pro 7+',
  'surface-pro8': 'Surface Pro 8',
  'surface-go': 'Surface GO',
  'surface-go2': 'Surface GO 2',
  'surface-go3': 'Surface GO 3',
  'eve_jemaos': 'Pixelbook',
  'kukui_jemaos': 'Jematab Duo',
  'one-a1': 'One A1',
  'rpi4-jemaos': 'Raspberry Pi 400',
  'rpi5-jemaos': 'Raspberry Pi 5',
  'orangepi5-jemaos': 'Orange Pi 5',
  'rock5b-jemaos': 'Rock 5B',
  'jematab_duo-jemaos': 'Jematab Duo',
};

export const JemaOSBoardNameTitleListWithI18n = [{
  board: 'amd64-jemaos',
  key: 'aboutJemaOSDeviceTitleLegacyIntel',
  fallback: 'Legacy Intel',
}, {
  board: 'amd64-jemaos_iris',
  key: 'aboutJemaOSDeviceTitleModernIntel',
  fallback: 'Modern Intel',
}, {
  board: 'amd64-jemaos_apu',
  key: 'aboutJemaOSDeviceTitleAMDGraphics',
  fallback: 'AMD Graphics',
}, {
  board: 'amd64-jemaos_slim',
  key: 'aboutJemaOSDeviceTitleIntelSlim',
  fallback: 'Intel Slim',
}];

export const JemaOSBoardNameReleaseNameMap: {[index: string]: string} = {
  'amd64-jemaos': 'for PC',
  'amd64-jemaos_iris': 'for PC',
  'amd64-jemaos_apu': 'for PC',
  'amd64-jemaos_slim': 'for PC',
  'amd64-vmware': 'for VMware',
  'rpi4-jemaos': 'for SBC',
  'rpi5-jemaos': 'for SBC',
  'rock5b-jemaos': 'for SBC',
  'orangepi5-jemaos': 'for SBC',
  'jematab_duo-jemaos': '-',
}
