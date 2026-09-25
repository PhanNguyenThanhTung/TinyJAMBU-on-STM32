import pytest
from pytest_embedded_idf.dut import IdfDut
from pytest_embedded_idf.utils import idf_parametrize


@pytest.mark.generic
@idf_parametrize('target', ['esp32s3'], indirect=['target'])
def test_receiver_starts(dut: IdfDut) -> None:
    dut.expect('TinyJAMBU UART receiver ready')
